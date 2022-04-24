#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>

#define DEBUG

using namespace std;

class Patient
{
public:
    Patient() = default;
    Patient(const vector<char*>& cellPtrs)
    {
        id = *(cellPtrs[0]);
        name = cellPtrs[1];
        surname = cellPtrs[2];
        pesel = cellPtrs[3];
    }
    void print() const
    {
        cout << "Id: " << id << " Imię: " << name
            << " Nazwisko: " << surname << " Pesel: " << pesel << endl;
    }
    SQLSMALLINT id;
    string name;
    string surname;
    string pesel;
};

class Study
{
public:
   	SQLSMALLINT patient_id;
    SQLCHAR type[20];
    SQLCHAR date[20];
    SQLCHAR result[20];
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

    template <typename T>
    vector<T> getQueryResult(const string& command, const vector<size_t>& cellSizes)
    {
        SQLLEN len;
        Patient patient;

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


        vector<T> objects;
        retcode = SQLFetch(hstmt);
        while (retcode != SQL_NO_DATA)
        {
            T obj(cellPtrs);
            objects.push_back(obj);
            retcode = SQLFetch(hstmt);
        }
        retcode = SQLFreeStmt(hstmt, SQL_DROP);
        return objects;
    }
    void selectFromStudies()
    {
        SQLLEN len;
        Study study;

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string command = "select * from studies";
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)command.c_str(), SQL_NTS);
        checkError(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 1, SQL_C_USHORT, &study.patient_id, 2, &len);
        checkError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, &study.type, 20, &len);
        checkError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 3, SQL_C_CHAR, &study.date, 20, &len);
        checkError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, &study.result, 20, &len);
        checkError(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLFetch(hstmt);
        while (retcode != SQL_NO_DATA)
        {
            cout << "Patient id: " << study.patient_id << " Nazwa badania: " << study.type
                << " Data: " << study.date << " Rezultat: " << study.result << endl;
            retcode = SQLFetch(hstmt);
        }
        retcode = SQLFreeStmt(hstmt, SQL_DROP);
    }
    void insertPatient(Patient patient)
    {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string komenda = "INSERT INTO Patients (name, surname, pesel) VALUES ('"+patient.name+ "','" +patient.surname+"','"+patient.pesel+"');";
        cout << "Komenda: " << komenda << endl;
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)komenda.c_str(), SQL_NTS);
        checkError(retcode, "SQLExecDirect() INSERT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLFreeStmt(hstmt, SQL_DROP);
    }
    void insertStudy(Study study)
    {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        checkError(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string komenda = "INSERT INTO Studies (patient_id, type, date, result) VALUES ('"+to_string(study.patient_id)+"','"+string((char*)study.type)+"','"+string((char*)study.date)+"','"+string((char*)study.result)+"');";
        cout << "Komenda: " << komenda << endl;
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)komenda.c_str(), SQL_NTS);
        checkError(retcode, "SQLExecDirect() INSERT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLFreeStmt(hstmt, SQL_DROP);
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

    int option;
    while (1)
    {
        cout << "Co chcesz zrobic?" << endl
             << "1 - Insert\n2 - Select\n3 - Wyjdz" << endl;
        cin >> option;
        if (option == 1)
        {
            string tablica;
            cout << "Podaj tablicę: ";
            cin >> tablica;
            if (tablica == "Patients") {
                Patient patient = createPatient();
                db.insertPatient(patient);
            }
            else if (tablica == "Studies") {
                Study study = createStudy();
                db.insertStudy(study);
            }
            else {
                cout << "Nie podano prawidlowej tablicy więc wybrana zostaje tablica Patients" << endl;
                Patient patient = createPatient();
                db.insertPatient(patient);
            }
        }
        else if (option == 2)
        {
            string tablica;
            cout << "Podaj tablicę: ";
            cin >> tablica;
            if (tablica == "Patients") {
                //db.selectFromPatients();
            }
            else if (tablica == "Studies") {
                db.selectFromStudies();
            }
            else {
                cout << "Nie podano prawidlowej tablicy więc wybrana zostaje tablica Patients" << endl;
                auto patients = db.getQueryResult<Patient>("select * from patients", {sizeof(Patient::id), 20, 20, 12});
                for (const auto& patient : patients) {
                    patient.print();
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
