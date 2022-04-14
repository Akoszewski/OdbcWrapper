#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string.h>

using namespace std;

#define CHECK_ERROR(code, str, h, env)  \
    if (code != SQL_SUCCESS)            \
    {                                   \
        extract_error(str, h, env);     \
        clearHandle(hstmt, hdbc, henv); \
        exit(0);                        \
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

void clearHandle(SQLHSTMT hstmt, SQLHDBC hdbc, SQLHENV henv)
{
    if (hstmt != SQL_NULL_HSTMT)
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc != SQL_NULL_HDBC)
    {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    if (henv != SQL_NULL_HENV)
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

class Patient
{
public:
    SQLSMALLINT id;
    SQLCHAR name[20];
    SQLCHAR surname[20];
    SQLCHAR pesel[11];
};

class Study
{
public:
   	SQLSMALLINT patient_id;
    SQLCHAR type[20];
    SQLCHAR date[20];
    SQLCHAR result[20];
};

class DbManager
{
public:
    DbManager(string dbname)
      : henv(SQL_NULL_HENV), hdbc(SQL_NULL_HDBC), hstmt(SQL_NULL_HSTMT)
    {
        retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
        CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_ENV)", henv, SQL_HANDLE_ENV);
        retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER *)SQL_OV_ODBC3, 0);
        CHECK_ERROR(retcode, "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)", henv, SQL_HANDLE_ENV);
        retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
        CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_DBC)", hdbc, SQL_HANDLE_DBC);
        retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
        CHECK_ERROR(retcode, "SQLSetConnectAttr(SQL_LOGIN_TIMEOUT)", hdbc, SQL_HANDLE_DBC);

        retcode = SQLConnect(hdbc, (SQLCHAR *)dbname.c_str(), SQL_NTS, (SQLCHAR *)NULL, SQL_NTS, NULL, SQL_NTS);
        CHECK_ERROR(retcode, "SQLConnect(DATASOURCE)", hdbc, SQL_HANDLE_DBC);
    }

    ~DbManager() 
    {
        clearHandle(hstmt, hdbc, henv);
    }

    void selectFromPatients()
    {
        SQLLEN len;
        Patient patient;

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string command = "select * from patients";
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)command.c_str(), SQL_NTS);
        CHECK_ERROR(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 1, SQL_C_USHORT, &patient.id, 2, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, &patient.name, 20, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 3, SQL_C_CHAR, &patient.surname, 20, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, &patient.pesel, 11, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLFetch(hstmt);
        while (retcode != SQL_NO_DATA)
        {
            cout << "Id: " << patient.id << " Imię: " << patient.name
                << " Nazwisko: " << patient.surname << " Pesel: " << patient.pesel << endl;
            retcode = SQLFetch(hstmt);
        }
        retcode = SQLFreeStmt(hstmt, SQL_DROP);
    }
    void selectFromStudies()
    {
        SQLLEN len;
        Study study;

        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string command = "select * from studies";
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)command.c_str(), SQL_NTS);
        CHECK_ERROR(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 1, SQL_C_USHORT, &study.patient_id, 2, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, &study.type, 20, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 3, SQL_C_CHAR, &study.date, 20, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

        retcode = SQLBindCol(hstmt, 4, SQL_C_CHAR, &study.result, 20, &len);
        CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);

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
        CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string komenda = "INSERT INTO Patients (name, surname, pesel) VALUES ('"+string((char*)patient.name)+ "','" +string((char*)patient.surname)+"','"+string((char*)patient.pesel)+"');";
        cout << "Komenda: " << komenda << endl;
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)komenda.c_str(), SQL_NTS);
        CHECK_ERROR(retcode, "SQLExecDirect() INSERT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLFreeStmt(hstmt, SQL_DROP);
    }
    void insertStudy(Study study)
    {
        retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);

        string komenda = "INSERT INTO Studies (patient_id, type, date, result) VALUES ('"+to_string(study.patient_id)+"','"+string((char*)study.type)+"','"+string((char*)study.date)+"','"+string((char*)study.result)+"');";
        cout << "Komenda: " << komenda << endl;
        retcode = SQLExecDirect(hstmt, (SQLCHAR *)komenda.c_str(), SQL_NTS);
        CHECK_ERROR(retcode, "SQLExecDirect() INSERT", hstmt, SQL_HANDLE_STMT);

        retcode = SQLFreeStmt(hstmt, SQL_DROP);
    }
private:
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLRETURN retcode;
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
                db.selectFromPatients();
            }
            else if (tablica == "Studies") {
                db.selectFromStudies();
            }
            else {
                cout << "Nie podano prawidlowej tablicy więc wybrana zostaje tablica Patients" << endl;
                db.selectFromPatients();
            }
        }
        else
        {
            exit(0);
        }
    }
    return 0;
}
