// deplib.h
// Headerfile
// H.Teufelhart
#pragma once

#define XMAS_CODEPAGE 850
#define XMAS_MAX_CHAR 5000

#define XMAS_FT_URL      L"http://localhost:8524/438BE08C-1D87-440D-A4F0-A21A337C5202"
#define XMAS_CASHBOX_ID  L"f9bb4d9f-db98-4c24-a614-87f9d874f0cc"
//#define XMAS_CASHBOX_ID  L"f9bb4d9f-db98-4c24-a614-87f9d874f0cd"
#define XMAS_TERMINAL_ID L"1"


int deplib_init(void);
int deplib_send_hello(char *Hellotext);
int deplib_sign(char *Rechnungstext, char *Zahlungsart, double Rechnungssumme, double MWST, char Signature[] );
int deplib_sign_null(char Signature[]);
int deplib_sign_startbeleg(char Signature[]);
int deplib_sign_stopbeleg(char Signature[]);
int deplib_sign_monatsbeleg(char Signature[]);  // funktioniert derzeit nicht
int deplib_get_dep(int *len, char *dep_result[]);
int deplib_exit(void);
