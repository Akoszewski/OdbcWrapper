#include<iostream> //biblioteka portow wejscia/wyjscia
#include<sql.h>
#include<sqlext.h> //dwie biblioteki do obslugi sql
#include<string.h> //bibloteka do obslugi lancuchow znakow

using namespace std;

#define CHECK_ERROR(code, str, h, env) if (code!=SQL_SUCCESS) {extract_error(str, h, env); clearHandle(hstmt, hdbc, henv); return 0;}

void extract_error(string fn, SQLHANDLE handle, SQLSMALLINT type){
	SQLINTEGER i = 0;
	SQLINTEGER native;
	SQLCHAR state[ 7 ];
	SQLCHAR text[256];
	SQLSMALLINT len;
	SQLRETURN ret;
	printf("\nThe driver reported the following diagnostics whilst running %s\n\n", fn.c_str());
	do
	{
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len );
		printf("\nSQLGetDiagRec returned %d\n\r", ret);
		if (SQL_SUCCEEDED(ret))
			printf("%s:%ld:%ld:%s\n", state, (long)i, (long)native, text);
	}
	while( ret == SQL_SUCCESS );
}

void clearHandle(SQLHSTMT hstmt, SQLHDBC hdbc, SQLHENV henv){
	if (hstmt != SQL_NULL_HSTMT)
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (hdbc != SQL_NULL_HDBC) {
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	}
	if (henv != SQL_NULL_HENV)
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

int main(){ //poczatek glownej funkcji programu
   //laczenie sie z baz:
	//deklaracje zmiennych:
	SQLHENV henv = SQL_NULL_HENV;  
	SQLHDBC hdbc = SQL_NULL_HDBC; 
	SQLHSTMT hstmt = SQL_NULL_HSTMT; //uchwyty do laczenia sie z odbc
	SQLRETURN retcode;

   //funkcje:
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_ENV)", henv, SQL_HANDLE_ENV);
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	CHECK_ERROR(retcode, "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION)", henv, SQL_HANDLE_ENV);
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc); 
	CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_DBC)", hdbc, SQL_HANDLE_DBC);
	retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	CHECK_ERROR(retcode, "SQLSetConnectAttr(SQL_LOGIN_TIMEOUT)", hdbc, SQL_HANDLE_DBC);

	retcode = SQLConnect(hdbc, (SQLCHAR*) "Lab2 ODBC", SQL_NTS, (SQLCHAR*) NULL, SQL_NTS, NULL, SQL_NTS);
	CHECK_ERROR(retcode, "SQLConnect(DATASOURCE)", hdbc, SQL_HANDLE_DBC);

   //obsluga instrukcji wysylanych do bazy danych:	
	SQLCHAR strNazwa[20];
	SQLLEN lenNazwa=0, cId=0, lenId=0; //deklaracje zmiennych na ktore bobierane beda dane z bazy
	int i;
	string komenda, nazwaKont;

	while(1){	
		cout<<"Co chcesz zrobic?"<<endl<<"1 - Insert\n2 - Select\n3 - Wyjdz"<<endl;
		cin>>i;
		if(i==1){
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
			CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);	
			cout<<"Podaj nazwe kontynentu: ";
			cin>>nazwaKont;
			string komenda = "INSERT INTO kontynenty (nazwa) VALUES ('"+nazwaKont+"')";
			retcode = SQLExecDirect(hstmt,(SQLCHAR*) komenda.c_str(), SQL_NTS);
			CHECK_ERROR(retcode, "SQLExecDirect() INSERT", hstmt, SQL_HANDLE_STMT);
			retcode = SQLFreeStmt(hstmt, SQL_DROP);
		}
		else if(i==2){		
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
			CHECK_ERROR(retcode, "SQLAllocHandle(SQL_HANDLE_STMT)", hstmt, SQL_HANDLE_STMT);			
			retcode = SQLExecDirect(hstmt,(SQLCHAR*) "SELECT id, nazwa FROM kontynenty", SQL_NTS);
			CHECK_ERROR(retcode, "SQLExecDirect() SELECT", hstmt, SQL_HANDLE_STMT);
			//wywolywanie dowolnej instrukcji sql, w tym wypadku SELECT 	

			retcode = SQLBindCol(hstmt, 1, SQL_C_USHORT, &cId,      2,  &lenId);
			CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);	
			retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR,   &strNazwa,   20, &lenNazwa);
			CHECK_ERROR(retcode, "SQLBindCol()", hstmt, SQL_HANDLE_STMT);	
        		//podpinanie pod kolumny zwracane przez select wybranych zmiennych
			
			retcode = SQLFetch(hstmt); //pobranie pojedynczego wiersza zwracanego po select
			while(retcode != SQL_NO_DATA) {
				cout<<"Id: "<<cId<<", Nazwa: "<<strNazwa<<endl; 
				//wyswietlanie na ekranie pobranych wartosci
				retcode = SQLFetch(hstmt); //pobranie kolejnego wiersza
			}
			retcode = SQLFreeStmt(hstmt, SQL_DROP);
		}
		else{
			clearHandle(hstmt, hdbc, henv);
			return 0;
		}
	}
	return 0;
}
