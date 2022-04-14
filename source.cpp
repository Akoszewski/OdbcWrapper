#include<iostream> //biblioteka portow wejscia/wyjscia
#include<sql.h>
#include<sqlext.h> //dwie biblioteki do obslugi sql
#include<string.h> //bibloteka do obslugi lancuchow znakow

using namespace std;

int main(){ //poczatek glownej funkcji programu
   //laczenie sie z baz:
	//deklaracje zmiennych:
	SQLHENV henv = SQL_NULL_HENV;  
	SQLHDBC hdbc = SQL_NULL_HDBC; 
	SQLHSTMT hstmt = SQL_NULL_HSTMT; //uchwyty do laczenia sie z odbc
	SQLHSTMT hstmt2 = SQL_NULL_HSTMT;
	//funkcje:
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc); 
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	SQLConnect(hdbc, (SQLCHAR*) "Lab2 ODBC", SQL_NTS, (SQLCHAR*) NULL, SQL_NTS, NULL, SQL_NTS);

   //obsluga instrukcji wysylanych do bazy danych:	
	SQLCHAR strNazwa[20];
	SQLLEN lenNazwa=0, cId=0, lenId=0; //deklaracje zmiennych na ktore bobierane beda dane z bazy
	SQLRETURN retcode;
	int i;
	string komenda, nazwaKont;

	while(1){	
		cout<<"Co chcesz zrobic?"<<endl<<"1 - Insert\n2 - Select\n3 - Wyjdz"<<endl;
		cin>>i;
		if(i==1){
			SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);	
			cout<<"Podaj nazwe kontynentu: ";
			cin>>nazwaKont;
			string komenda = "INSERT INTO kontynenty (nazwa) VALUES ('"+nazwaKont+"')";
			SQLExecDirect(hstmt,(SQLCHAR*) komenda.c_str(), SQL_NTS);
			SQLFreeStmt(hstmt, SQL_DROP);
		}
		else if(i==2){		
			SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);			
			retcode = SQLExecDirect(hstmt,(SQLCHAR*) "SELECT id, nazwa FROM kontynenty", SQL_NTS);
			//wywolywanie dowolnej instrukcji sql, w tym wypadku SELECT 	

			SQLBindCol(hstmt, 1, SQL_C_USHORT, &cId,      2,  &lenId);
			SQLBindCol(hstmt, 2, SQL_C_CHAR,   &strNazwa,   20, &lenNazwa);
        		//podpinanie pod kolumny zwracane przez select wybranych zmiennych
			
			retcode = SQLFetch(hstmt); //pobranie pojedynczego wiersza zwracanego po select
			while(retcode != SQL_NO_DATA) {
				cout<<"Id: "<<cId<<", Nazwa: "<<strNazwa<<endl; 
				//wyswietlanie na ekranie pobranych wartosci
				retcode = SQLFetch(hstmt); //pobranie kolejnego wiersza
			}
			SQLFreeStmt(hstmt, SQL_DROP);
		}
		else 
			return 0;
	}
	return 0;
}
