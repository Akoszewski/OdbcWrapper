#ifndef ODBC_WRAP
#define ODBC_WRAP

#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <list>
#include <memory>

class DbItem
{
public:
    DbItem() = default;
    DbItem(const std::vector<std::unique_ptr<char[]>>& cellPtrs){};
    virtual void print() const = 0;
};

class StringRow : public DbItem
{
public:
    StringRow() = default;
    StringRow(const std::vector<std::unique_ptr<char[]>>& cellPtrs)
    {
        for (const auto& cell : cellPtrs) {
            data.push_back(cell.get());
        }
    }
    void print() const override
    {
        for (const auto& cell : data) {
            std::cout << cell;
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    std::vector<std::string> data;
};

class ODBCWrap
{
public:
    ODBCWrap(std::string dbname)
      : henv(SQL_NULL_HENV), hdbc(SQL_NULL_HDBC), hstmt(SQL_NULL_HSTMT)
    {
        retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_ENV)", henv, SQL_HANDLE_ENV);
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER *)SQL_OV_ODBC3, 0);
        checkError(retcode, "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)", henv, SQL_HANDLE_ENV);
        retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_DBC)", hdbc, SQL_HANDLE_DBC);
        retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
        checkError(retcode, "SQLSetConnectAttr(SQL_LOGIN_TIMEOUT)", hdbc, SQL_HANDLE_DBC);

        retcode = SQLConnect(hdbc, (SQLCHAR *)dbname.c_str(), SQL_NTS, (SQLCHAR *)NULL, SQL_NTS, NULL, SQL_NTS);
        checkError(retcode, "SQLConnect(DATASOURCE)", hdbc, SQL_HANDLE_DBC);
    }

    ~ODBCWrap() 
    {
        freeResources();
    }

    template <typename T = StringRow>
    std::list<T> executeQuery(const std::string& command, const std::vector<size_t>& cellSizes = {})
    {
        SQLLEN len;

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        retcode = SQLExecDirect(hstmt, (SQLCHAR *)command.c_str(), SQL_NTS);
        checkError(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);

        std::vector<std::unique_ptr<char[]>> cellPtrs;
        cellPtrs.reserve(cellSizes.size());
        for (int i = 0; i < cellSizes.size(); ++i)
        {
            cellPtrs.emplace_back(new char[cellSizes[i]]);
            retcode = SQLBindCol(hstmt, i+1, SQL_C_CHAR, cellPtrs[i].get(), cellSizes[i], &len);
            checkError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);
        }

        std::list<T> objects;
        if (!cellSizes.empty())
        {
            retcode = SQLFetch(hstmt);
            while (retcode != SQL_NO_DATA)
            {
                objects.emplace_back(cellPtrs);
                retcode = SQLFetch(hstmt);
            }
            retcode = SQLFreeStmt(hstmt, SQL_DROP);
        }
        return objects;
    }
private:
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN retcode;

    void checkError(SQLRETURN code, const char* str, SQLHANDLE h, SQLSMALLINT env)
    {
        if (code != SQL_SUCCESS)
        {
            extract_error(str, h, env);
            freeResources();
            exit(0); 
        }
    }
    void freeResources()
    {
        if (hstmt != SQL_NULL_HSTMT) {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        }
        if (hdbc != SQL_NULL_HDBC)
        {
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        }
        if (henv != SQL_NULL_HENV) {
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
        }
    }
    void extract_error(std::string fn, SQLHANDLE handle, SQLSMALLINT type)
    {
        SQLINTEGER i = 0;
        SQLINTEGER native;
        SQLCHAR state[7];
        SQLCHAR text[256];
        SQLSMALLINT len;
        SQLRETURN ret;
        printf("\nThe driver reported the following diagnostics whilst running %s\n\n", fn.c_str());
        do
        {
            ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
            printf("\nSQLGetDiagRec returned %d\n\r", ret);
            if (SQL_SUCCEEDED(ret))
                printf("%s:%ld:%ld:%s\n", state, (long)i, (long)native, text);
        } while (ret == SQL_SUCCESS);
    }
};


class Patient : public DbItem
{
public:
    Patient() = default;
    Patient(const std::vector<std::unique_ptr<char[]>>& cellPtrs)
    {
        id = atoi(const_cast<const char*>(cellPtrs[0].get()));
        name = cellPtrs[1].get();
        surname = cellPtrs[2].get();
        pesel = cellPtrs[3].get();
    }
    void print() const override
    {
        std::cout << "Id: " << id << " Imię: " << name
            << " Nazwisko: " << surname << " Pesel: " << pesel << std::endl;
    }
    SQLSMALLINT id;
    std::string name;
    std::string surname;
    std::string pesel;
};

class Study : public DbItem
{
public:
    Study() = default;
    Study(const std::vector<std::unique_ptr<char[]>>& cellPtrs)
    {
        patient_id = atoi(const_cast<const char*>(cellPtrs[0].get()));
        type = cellPtrs[1].get();
        date = cellPtrs[2].get();
        result = cellPtrs[3].get();
    }
    void print() const override
    {
        std::cout << "Patient id: " << patient_id << " Nazwa badania: " << type
            << " Data: " << date << " Rezultat: " << result << std::endl;
    }
   	SQLSMALLINT patient_id;
    std::string type;
    std::string date;
    std::string result;
};


# endif


Patient createPatient()
{
    Patient patient;
    std::cout << "Podaj imię pacjenta: ";
    std::cin >> patient.name;
    std::cout << "Podaj nazwisko pacjenta: ";
    std::cin >> patient.surname;
    std::cout << "Podaj pesel pacjenta: ";
    std::cin >> patient.pesel;
    return patient;
}

Study createStudy()
{
    Study study;
    std::cout << "Podaj id pacjenta: ";
    std::cin >> study.patient_id;
    std::cout << "Podaj nazwę badania: ";
    std::cin >> study.type;
    std::cout << "Podaj datę badania: ";
    std::cin >> study.date;
    std::cout << "Podaj rezultat badania: ";
    std::cin >> study.result;
    return study;
}

int main()
{
    ODBCWrap odbc("Lab1 ODBC");

    while (true)
    {
        char tabela;
        // auto tabele = db.executeQuery<>("SELECT * FROM information_schema.tables", {128, 128, 128, 128, 128, 128, 128});
        // for (const auto& study : tabele) {
        //     study.print();
        // }
        std::cout << "\nWybierz tabele:" << std::endl
            << "1 - Patients\n2 - Studies\n3 - Wyjdz" << std::endl;
        std::cin >> tabela;
        if (tabela == '1')
        {
            while (true) {
                char option;
                std::cout << "\nCo chcesz zrobic?" << std::endl
                     << "1 - Insert\n2 - Select\n3 - Powrot" << std::endl;
                std::cin >> option;
                if (option == '1') {
                    Patient patient = createPatient();
                    std::string komenda = "INSERT INTO Patients (name, surname, pesel) VALUES ('"+patient.name+ "','" +patient.surname+"','"+patient.pesel+"');";
                    odbc.executeQuery<>(komenda);
                }
                else if (option == '2') {
                    auto patients = odbc.executeQuery<Patient>("select * from patients", {sizeof(Patient::id), 20, 20, 12});
                    for (const auto& patient : patients) {
                        patient.print();
                    }
                }
                else {
                    break;
                }
            }
        }
        else if (tabela == '2')
        {
            while (true) {
                char option;
                std::cout << "\nCo chcesz zrobic?" << std::endl
                     << "1 - Insert\n2 - Select\n3 - Powrot" << std::endl;
                std::cin >> option;
                if (option == '1') {
                    Study study = createStudy();
                    std::string komenda = "INSERT INTO Studies (patient_id, type, date, result) VALUES ('"+std::to_string(study.patient_id)+"','"+study.type+"','"+study.date+"','"+study.result+"');";
                    odbc.executeQuery<>(komenda);
                }
                else if (option == '2') {
                    auto studies = odbc.executeQuery<Study>("select * from Studies", {sizeof(Study::patient_id), 20, 20, 20});
                    for (const auto& study : studies) {
                        study.print();
                    }
                }
                else {
                    break;
                }
            }
        }
        else
        {
            break;
        }
    }
    return 0;
}
