#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <list>

// #define DEBUG

using namespace std;

class DbItem
{
public:
    DbItem() = default;
    DbItem(const vector<char*>& cellPtrs){};
    virtual void print() const {};
};

class Patient : public DbItem
{
public:
    Patient() = default;
    Patient(const vector<char*>& cellPtrs)
    {
        id = atoi(const_cast<const char*>(cellPtrs[0]));
        name = cellPtrs[1];
        surname = cellPtrs[2];
        pesel = cellPtrs[3];
    }
    void print() const override
    {
        cout << "Id: " << id << " Imię: " << name
            << " Nazwisko: " << surname << " Pesel: " << pesel << endl;
    }
    SQLSMALLINT id;
    string name;
    string surname;
    string pesel;
};

class Study : public DbItem
{
public:
    Study() = default;
    Study(const vector<char*>& cellPtrs)
    {
        patient_id = atoi(const_cast<const char*>(cellPtrs[0]));
        type = cellPtrs[1];
        date = cellPtrs[2];
        result = cellPtrs[3];
    }
    void print() const override
    {
        cout << "Patient id: " << patient_id << " Nazwa badania: " << type
            << " Data: " << date << " Rezultat: " << result << endl;
    }
   	SQLSMALLINT patient_id;
    string type;
    string date;
    string result;
};

#ifdef DEBUG

void* operator new(size_t size)
{
    cout << "Alokacja: " << size << endl;
    return malloc(size);
}

void operator delete(void* ptr)
{
    cout << "Dealokacja: " << endl;
    free(ptr);
}

# endif

class DbManager
{
public:
    DbManager(string dbname)
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

    ~DbManager() 
    {
        freeResources();
    }

    template <typename T = DbItem>
    list<T> executeQuery(const string& command, const vector<size_t>& cellSizes = {})
    {
        SQLLEN len;

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        retcode = SQLExecDirect(hstmt, (SQLCHAR *)command.c_str(), SQL_NTS);
        checkError(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);

        vector<char*> cellPtrs;
        for (int i = 0; i < cellSizes.size(); ++i)
        {
            cellPtrs.push_back(new char[cellSizes[i]]);
            retcode = SQLBindCol(hstmt, i+1, SQL_C_CHAR, cellPtrs[i], cellSizes[i], &len);
            checkError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);
        }

        list<T> objects;
        if (!cellSizes.empty()) 
        {
            retcode = SQLFetch(hstmt);
            while (retcode != SQL_NO_DATA)
            {
                T obj(cellPtrs);
                objects.push_back(obj);
                retcode = SQLFetch(hstmt);
            }
            retcode = SQLFreeStmt(hstmt, SQL_DROP);
        }
        for (const auto& el : cellPtrs) {
            delete el;
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
    void extract_error(string fn, SQLHANDLE handle, SQLSMALLINT type)
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

Patient createPatient()
{
    Patient patient;
    cout << "Podaj imię pacjenta: ";
    cin >> patient.name;
    cout << "Podaj nazwisko pacjenta: ";
    cin >> patient.surname;
    cout << "Podaj pesel pacjenta: ";
    cin >> patient.pesel;
    return patient;
}

Study createStudy()
{
    Study study;
    cout << "Podaj id pacjenta: ";
    cin >> study.patient_id;
    cout << "Podaj nazwę badania: ";
    cin >> study.type;
    cout << "Podaj datę badania: ";
    cin >> study.date;
    cout << "Podaj rezultat badania: ";
    cin >> study.result;
    return study;
}

int main()
{
    DbManager db("Lab1 ODBC");

    while (true)
    {
        char tabela;
        cout << "\nWybierz tabele:" << endl
            << "1 - Patients\n2 - Studies\n3 - Wyjdz" << endl;
        cin >> tabela;
        if (tabela == '1')
        {
            while (true) {
                char option;
                cout << "\nCo chcesz zrobic?" << endl
                     << "1 - Insert\n2 - Select\n3 - Powrot" << endl;
                cin >> option;
                if (option == '1') {
                    Patient patient = createPatient();
                    string komenda = "INSERT INTO Patients (name, surname, pesel) VALUES ('"+patient.name+ "','" +patient.surname+"','"+patient.pesel+"');";
                    db.executeQuery<>(komenda);
                }
                else if (option == '2') {
                    auto patients = db.executeQuery<Patient>("select * from patients", {sizeof(Patient::id), 20, 20, 12});
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
                cout << "\nCo chcesz zrobic?" << endl
                     << "1 - Insert\n2 - Select\n3 - Powrot" << endl;
                cin >> option;
                if (option == '1') {
                    Study study = createStudy();
                    string komenda = "INSERT INTO Studies (patient_id, type, date, result) VALUES ('"+to_string(study.patient_id)+"','"+study.type+"','"+study.date+"','"+study.result+"');";
                    db.executeQuery<>(komenda);
                }
                else if (option == '2') {
                    auto studies = db.executeQuery<Study>("select * from Studies", {sizeof(Study::patient_id), 20, 20, 20});
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
