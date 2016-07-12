// deplib.cpp
// H.Teufelhart XMAS 2016
// Grundlage dieser Demo: github steininger fiskaltrust - demo  cppConsoleApplicationSOAP
// Version 0.2  2016-07-15

#include "stdafx.h"
#include <ctime>
#include <WebServices.h>
#include <iostream>
#include <comdef.h>
using namespace std;
#include "fiskaltrust.interface.wsdl.h"

extern "C" 
{
#include "deplib.h"
}

void print_response(char *Signature);
void print_error();

// Diverse Konvertierungsroutinen um die .NET Datenformate zu erzeugen
// ACHTUNG bei allen string Konvertierungen wird Speicher angefordert, muss wieder freigegeben werden mit delete[] !
DECIMAL   conv_dbl2dec(double dblvalue)
{
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_R8;
	var.dblVal = dblvalue;
	VariantChangeType(&var, &var, 0, VT_DECIMAL);
	return var.decVal;
}
double    conv_dec2dbl(DECIMAL decvalue)
{
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_DECIMAL;
	var.decVal = decvalue;
	VariantChangeType(&var, &var, 0, VT_R8);
	return var.dblVal;
}
WCHAR *   conv_chr2wchr(char *chrvalue)
{
	int len = strlen(chrvalue);
	if (len > XMAS_MAX_CHAR)
		len = XMAS_MAX_CHAR;
	int wchr_cnt = MultiByteToWideChar(XMAS_CODEPAGE, 0, chrvalue, len + 1, NULL, 0);
	wchar_t *wchr = new wchar_t[wchr_cnt];
	MultiByteToWideChar(XMAS_CODEPAGE, 0, chrvalue, len + 1, wchr, wchr_cnt);
	return wchr;
}
char *    conv_wchr2chr(WCHAR *wchrvalue)
{
	int len = wcslen(wchrvalue);
	if (len > XMAS_MAX_CHAR)
		len = XMAS_MAX_CHAR;
	int chr_cnt = WideCharToMultiByte(XMAS_CODEPAGE, 0, wchrvalue, len+1, NULL, 0, NULL, NULL);
	char *chr = new char[chr_cnt];
	WideCharToMultiByte(XMAS_CODEPAGE, 0, wchrvalue, len+1, chr, chr_cnt, NULL, NULL);
	return chr;
}
WS_STRING conv_chr2wsstr(char *chrvalue)
{
	WS_STRING wsstr;
	wsstr.chars = conv_chr2wchr(chrvalue);
	wsstr.length = wcslen(wsstr.chars);
	return wsstr;
}
char *    conv_wsstr2chr(WS_STRING wsstrvalue, char buf[] )
{
	char *chr;
	int len = wsstrvalue.length;
	if (len >= XMAS_MAX_CHAR - 1)
		len = XMAS_MAX_CHAR - 1;
	int chr_cnt = WideCharToMultiByte(XMAS_CODEPAGE, 0, wsstrvalue.chars, len + 1, NULL, 0, NULL, NULL);

	if (buf == NULL)
		chr = new char[chr_cnt];
	else
		chr = buf;

	WideCharToMultiByte(XMAS_CODEPAGE, 0, wsstrvalue.chars, len + 1, chr, chr_cnt, NULL, NULL);
	chr[len] = '\0';
	return chr;
}



#ifndef Release_LIB
int main()  //====================================================================================================================================================
{
	int ret;
	char buf_sign[XMAS_MAX_CHAR];

	ret = deplib_init();

	ret = deplib_send_hello("Hello2");

	ret = deplib_sign_startbeleg(buf_sign);		// Funktioniert nur beim ersten Mal
	printf("Startbeleg   Return: %i  Signatur: %s\n\n", ret, buf_sign);

	ret = deplib_sign_null(buf_sign);
	printf("Nullbeleg    Return: %i  Signatur: %s\n\n", ret, buf_sign);

	ret = deplib_sign("Rechnung 123", "Bar", 30.0, 20.0, buf_sign);
	printf("Signieren    Return: %i  Signatur: %s\n\n", ret, buf_sign);

	ret = deplib_sign_null(buf_sign);
	printf("Nullbeleg    Return: %i  Signatur: %s\n\n", ret, buf_sign);

	ret = deplib_sign_monatsbeleg(buf_sign);	// Funktioniert derzeit nicht
	printf("Monatsbeleg  Return: %i  Signatur: %s\n\n", ret, buf_sign);

//	ret = deplib_sign_stopbeleg(buf_sign);     // ACHTUNG der stopbeleg beendet die Möglichkeit zum Signieren, keine Möglichkeit den Dienst wieder zu aktivieren
//	printf("Stopbeleg    Return: %i  Signatur: %s\n\n", ret, buf_sign);

	int len;
	char *dep;
	ret = deplib_get_dep(&len, &dep);			// Funktioniert nur bis zu einer Länge von 65536 Bytes
	printf("-------- RSKV DEP ---- Start --------------------------------------------------\n");
	printf("%.*s\n", len, dep);
	printf("-------- RSKV DEP ---- Ende ---------------------------------------------------\n");
	printf("Get DEP      Return: %i  Lenght: %d\n\n", ret, len);

	ret = deplib_exit();

	printf("%s", conv_wchr2chr(L"Ende: irgendeine Taste druecken zum Beenden"));
	ret = getchar();

	return ret;
}
#endif

WS_SERVICE_PROXY* proxy = NULL;
WS_ERROR* error = NULL;
ReceiptResponse *din = { 0 };
WS_HEAP* heap = NULL;

#define WS_STRING_SET(ws, s) { ws.chars = s; ws.length = WsCountOf(s) - 1; }

extern "C" int deplib_init()  //=========================================================================================================================
{
	HRESULT hr = NOERROR;
	int ret = 0;

	//Fehlerspeicher vorbereiten
	hr = WsCreateError(NULL, 0, &error);

	//Nachrichtenspeicher vorbereiten; Unbedingt die maximale Nachrichtengröße (maxSize) auf 65536 setzen, da dies der Standardwert ist beim .NET 4
	hr = WsCreateHeap(/*maxSize*/ 65536, 512, NULL, 0, &heap, error);

	//Endpunkt vorbereiten
	WS_ENDPOINT_ADDRESS address = {};
//	WS_STRING url = WS_STRING_VALUE(L"http://localhost:1201/fiskaltrust/POS");  Version 0.1
	WS_STRING url = WS_STRING_VALUE(XMAS_FT_URL);
	address.url = url;


	//Proxy erstellen und öffnen
	proxy = NULL;
	WS_HTTP_BINDING_TEMPLATE templ = {};
	hr = BasicHttpBinding_IPOS_CreateServiceProxy(&templ, NULL, 0, &proxy, error);
	if (FAILED(hr))
		return -2;

	hr = WsOpenServiceProxy(proxy, &address, NULL, error);
	if (FAILED(hr))
	{
		ULONG errorCount;
		hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
		if (!FAILED(hr))
		{
			for (ULONG i = 0; i < errorCount; i++)
			{
				WS_STRING errorMessage;
				hr = WsGetErrorString(error, i, &errorMessage);
				if (!FAILED(hr))
				{
#ifndef Release_LIB
					wprintf(L"%.*s\n", errorMessage.length, errorMessage.chars);
#endif
				}
			}
		}
		return -1;
	}
	
	ret = deplib_send_hello("Inititialisierung des Dienstes abgeschlossen");
	return ret;
}

extern "C" int deplib_send_hello(char *hellotext)  //=====================================================================================================================
{
	HRESULT hr = NOERROR;
	//echo-Funktion, zum Verbindungstes und Vorwärmen des Dienstes falls dieser neu gestartet wurde
	//	WS_STRING echoInMessage = WS_STRING_VALUE(L"Hello World!                                                   ");
	WS_STRING echoInMessage;
	echoInMessage.length = strlen(hellotext);
	echoInMessage.chars = conv_chr2wchr(hellotext);
	WS_STRING echoOutMessage;

	hr = BasicHttpBinding_IPOS_Echo(proxy, echoInMessage, &echoOutMessage, heap, NULL, 0, NULL, error);
	if (!FAILED(hr))
	{
#ifndef Release_LIB
		wprintf(L"%.*s\n", echoOutMessage.length, echoOutMessage.chars);
#endif
		delete[] echoInMessage.chars;
		return 0;
	}
	else
	{
		ULONG errorCount;
		hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
		if (!FAILED(hr))
		{
			for (ULONG i = 0; i < errorCount; i++)
			{
				WS_STRING errorMessage;
				hr = WsGetErrorString(error, i, &errorMessage);
				if (!FAILED(hr))
				{
#ifndef Release_LIB
					wprintf(L"%.*s\n", errorMessage.length, errorMessage.chars);
#endif
				}
			}
		}
		return -4;
	}
}


extern "C" int deplib_sign(char *rtext, char *rbar, double rvalue, double rvat, char *Signature)  //===================================================================================================
{
	HRESULT hr = NOERROR;
	ReceiptRequest dout = { 0 };
	ChargeItem *data_CI_ptr[2] = { 0 };
	ChargeItem data_CI1 = { 0 };
	PayItem	*data_PI_ptr[2] = { 0 };
	PayItem	data_PI = { 0 };

	din = { 0 };

	Signature[0] = '\0';

//================================ ReceiptRequest
//	WS_STRING_SET(dout.ftCashBoxID, L"xmas_test1");  // Version 0.1
	WS_STRING_SET(dout.ftCashBoxID, XMAS_CASHBOX_ID);
	WS_STRING_SET(dout.cbTerminalID, XMAS_TERMINAL_ID);
//	WS_STRING_SET(dout.cbReceiptReference, L"RE-12345");

	FILETIME ft;
//	SYSTEMTIME st;
	WS_DATETIME wsdt;
	GetSystemTimeAsFileTime(&ft);
	WsFileTimeToDateTime(&ft, &wsdt, NULL);

	dout.cbReceiptMoment = wsdt;
//	dout.cbReceiptMoment.format = WS_DATETIME_FORMAT_LOCAL;
	dout.cbChargeItemsCount = 1;
	dout.cbChargeItems = data_CI_ptr;
	dout.cbPayItemsCount = 1;
	dout.cbPayItems = data_PI_ptr;
	dout.ftReceiptCase = 0x4154000000000000;
//	WS_STRING_SET(dout.ftReceiptCaseData, L"");
//	dout.cbReceiptAmount = NULL;
//	WS_STRING_SET(dout.cbUser, L"");
//	WS_STRING_SET(dout.cbArea, L"");
//	WS_STRING_SET(dout.cbCustomer, L"");
//	WS_STRING_SET(dout.cbSettlement, L"");

//================================ ChargeItem
	data_CI_ptr[0] = &data_CI1;
	data_CI1.Quantity = conv_dbl2dec(1.0);
	data_CI1.Description.length = strlen(rtext);
	data_CI1.Description.chars  = conv_chr2wchr(rtext);
	data_CI1.Amount = conv_dbl2dec(rvalue);
	data_CI1.VATRate = conv_dbl2dec(rvat);
	data_CI1.ftChargeItemCase= 0x4154000000000000;
//	WS_STRING_SET(data_CI1.ftChargeItemCaseData, L"");
//	data_CI1.VATAmount = NULL;
//	WS_STRING_SET(data_CI1.AccountNumber, L"");
//	WS_STRING_SET(data_CI1.CostCenter, L"");
//	WS_STRING_SET(data_CI1.ProductGroup, L"");
//	WS_STRING_SET(data_CI1.ProductNumber, L"");
//	WS_STRING_SET(data_CI1.ProductBarcode, L"");
//	WS_STRING_SET(data_CI1.Unit, L"");
//	data_CI1.UnitQuantity = NULL;
//	data_CI1.UnitPrice = NULL;
//	data_CI1.Moment = NULL;

//================================ PayItem
	data_PI_ptr[0] = &data_PI;
	data_PI.Quantity = conv_dbl2dec(1.0);
	data_PI.Description.length = strlen(rbar);
	data_PI.Description.chars = conv_chr2wchr(rbar);
	data_PI.Amount = conv_dbl2dec(rvalue);
	data_PI.ftPayItemCase= 0x4154000000000000;
//	WS_STRING_SET(data_PI.ftPayItemCaseData, L"");
//	WS_STRING_SET(data_PI.AccountNumber, L"");
//	WS_STRING_SET(data_PI.CostCenter, L"");
//	WS_STRING_SET(data_PI.MoneyGroup, L"");
//	WS_STRING_SET(data_PI.MoneyNumber, L"");
//	data_PI.Moment = NULL;

	hr = BasicHttpBinding_IPOS_Sign(proxy, &dout, &din, heap, NULL, 0, NULL, error);
	if (!FAILED(hr))
	{
		print_response(Signature);

		delete[] data_CI1.Description.chars;
		delete[] data_PI.Description.chars;
		return (int)din->ftState;
	}
	else
	{
		print_error();
		return -4;
	}
}

extern "C" int deplib_sign_null(char *Signature)     //=========================================================================================================================
{
	HRESULT hr = NOERROR;
	ReceiptRequest dout = { 0 };
	ChargeItem *data_CI_ptr[2] = { 0 };
	ChargeItem data_CI1 = { 0 };
	PayItem	*data_PI_ptr[2] = { 0 };
	PayItem	data_PI = { 0 };

	Signature[0] = '\0';

	//================================ ReceiptRequest
	WS_STRING_SET(dout.ftCashBoxID, XMAS_CASHBOX_ID);
	WS_STRING_SET(dout.cbTerminalID, XMAS_TERMINAL_ID);

	FILETIME ft;
	WS_DATETIME wsdt;
	GetSystemTimeAsFileTime(&ft);
	WsFileTimeToDateTime(&ft, &wsdt, NULL);

	dout.cbReceiptMoment = wsdt;
//	dout.cbReceiptMoment.format = WS_DATETIME_FORMAT_LOCAL;
	dout.cbChargeItemsCount = 0;
	dout.cbChargeItems = data_CI_ptr;
	dout.cbPayItemsCount = 0;
	dout.cbPayItems = data_PI_ptr;
	dout.ftReceiptCase = 0x4154000000000002;

	hr = BasicHttpBinding_IPOS_Sign(proxy, &dout, &din, heap, NULL, 0, NULL, error);
	if (!FAILED(hr))
	{
		print_response( Signature );
		return (int)din->ftState;
	}
	else
	{
		print_error();
		return -4;
	}
}

extern "C" int deplib_sign_startbeleg(char *Signature)     //=========================================================================================================================
{
	HRESULT hr = NOERROR;
	ReceiptRequest dout = { 0 };
	ChargeItem *data_CI_ptr[2] = { 0 };
	ChargeItem data_CI1 = { 0 };
	PayItem	*data_PI_ptr[2] = { 0 };
	PayItem	data_PI = { 0 };

	Signature[0] = '\0';

	//================================ ReceiptRequest
	WS_STRING_SET(dout.ftCashBoxID, XMAS_CASHBOX_ID);
	WS_STRING_SET(dout.cbTerminalID, XMAS_TERMINAL_ID);

	FILETIME ft;
	//	SYSTEMTIME st;
	WS_DATETIME wsdt;
	GetSystemTimeAsFileTime(&ft);
	WsFileTimeToDateTime(&ft, &wsdt, NULL);

	dout.cbReceiptMoment = wsdt;
	//	dout.cbReceiptMoment.format = WS_DATETIME_FORMAT_LOCAL;
	dout.cbChargeItemsCount = 0;
	dout.cbChargeItems = data_CI_ptr;
	dout.cbPayItemsCount = 0;
	dout.cbPayItems = data_PI_ptr;
	dout.ftReceiptCase = 0x4154000000000003;

	hr = BasicHttpBinding_IPOS_Sign(proxy, &dout, &din, heap, NULL, 0, NULL, error);
	if (din == NULL)
	{
#ifndef Release_LIB
		wprintf(L"!!!!!!!! null-result !!!!!!!!!\n");
		print_error();
#endif
		return -5;
	}

	if (!FAILED(hr))
	{
		print_response(Signature);
		return (int)din->ftState;
	}
	else
	{
		print_error();
		return -4;
	}
}

extern "C" int deplib_sign_stopbeleg(char *Signature)     //=========================================================================================================================
{
	HRESULT hr = NOERROR;
	ReceiptRequest dout = { 0 };
	ChargeItem *data_CI_ptr[2] = { 0 };
	ChargeItem data_CI1 = { 0 };
	PayItem	*data_PI_ptr[2] = { 0 };
	PayItem	data_PI = { 0 };

	Signature[0] = '\0';

	//================================ ReceiptRequest
	WS_STRING_SET(dout.ftCashBoxID, XMAS_CASHBOX_ID);
	WS_STRING_SET(dout.cbTerminalID, XMAS_TERMINAL_ID);

	FILETIME ft;
	//	SYSTEMTIME st;
	WS_DATETIME wsdt;
	GetSystemTimeAsFileTime(&ft);
	WsFileTimeToDateTime(&ft, &wsdt, NULL);

	dout.cbReceiptMoment = wsdt;
	//	dout.cbReceiptMoment.format = WS_DATETIME_FORMAT_LOCAL;
	dout.cbChargeItemsCount = 0;
	dout.cbChargeItems = data_CI_ptr;
	dout.cbPayItemsCount = 0;
	dout.cbPayItems = data_PI_ptr;
	dout.ftReceiptCase = 0x4154000000000004;

	hr = BasicHttpBinding_IPOS_Sign(proxy, &dout, &din, heap, NULL, 0, NULL, error);
	if (din == NULL)
	{
#ifndef Release_LIB
		wprintf(L"!!!!!!!! null-result !!!!!!!!!\n");
		print_error();
#endif
		return -5;
	}

	if (!FAILED(hr))
	{
		print_response(Signature);
		return (int)din->ftState;
	}
	else
	{
		print_error();
		return -4;
	}
}

extern "C" int deplib_sign_monatsbeleg(char *Signature)     //=========================================================================================================================
{
	HRESULT hr = NOERROR;
	ReceiptRequest dout = { 0 };
	ChargeItem *data_CI_ptr[2] = { 0 };
	ChargeItem data_CI1 = { 0 };
	PayItem	*data_PI_ptr[2] = { 0 };
	PayItem	data_PI = { 0 };

	Signature[0] = '\0';

	//================================ ReceiptRequest
	WS_STRING_SET(dout.ftCashBoxID, XMAS_CASHBOX_ID);
	WS_STRING_SET(dout.cbTerminalID, XMAS_TERMINAL_ID);

	FILETIME ft;
	//	SYSTEMTIME st;
	WS_DATETIME wsdt;
	GetSystemTimeAsFileTime(&ft);
	WsFileTimeToDateTime(&ft, &wsdt, NULL);

	dout.cbReceiptMoment = wsdt;
	//	dout.cbReceiptMoment.format = WS_DATETIME_FORMAT_LOCAL;
	dout.cbChargeItemsCount = 0;
	dout.cbChargeItems = data_CI_ptr;
	dout.cbPayItemsCount = 0;
	dout.cbPayItems = data_PI_ptr;
	dout.ftReceiptCase = 0x4154000000000005;

	hr = BasicHttpBinding_IPOS_Sign(proxy, &dout, &din, heap, NULL, 0, NULL, error);
	if (din == NULL)
	{
#ifndef Release_LIB
		wprintf(L"!!!!!!!! null-result !!!!!!!!!\n");
		print_error();
#endif
		return -5;
	}

	if (!FAILED(hr))
	{
		print_response(Signature);
		return (int)din->ftState;
	}
	else
	{
		print_error();
		return -4;
	}
}

extern "C" int deplib_get_dep(int *dep_len, char *dep_result[])     //=========================================================================================================================
{
	HRESULT hr = NOERROR;
	ReceiptRequest dout = { 0 };
	ChargeItem *data_CI_ptr[2] = { 0 };
	ChargeItem data_CI1 = { 0 };
	PayItem	*data_PI_ptr[2] = { 0 };
	PayItem	data_PI = { 0 };

	FILETIME ft;
	WS_DATETIME wsdt;
	GetSystemTimeAsFileTime(&ft);
	WsFileTimeToDateTime(&ft, &wsdt, NULL);

	static WS_BYTES dep_res;

	hr = BasicHttpBinding_IPOS_Journal(proxy, 0x4154000000000001, 0, wsdt.ticks, &dep_res, heap, NULL, 0, NULL, error);
	if (!FAILED(hr))
	{
//		wprintf(L"======================= RKSV-DEP =========================\n");
//		printf("%.*s\n", dep_res.length, (char *)dep_res.bytes );
		*dep_len = dep_res.length;
		*dep_result = (char *)dep_res.bytes;
		return 0;
	}
	else
	{
		print_error();
		return -4;
	}
}

void print_response(char *Signature)     //=========================================================================================================================
{
#ifndef Release_LIB
	wprintf(L"---------------------------------------------------------------------------\n");
	wprintf(L"ftReceiptID              %.*s\n", din->ftReceiptIdentification.length, din->ftReceiptIdentification.chars);

	wprintf(L"ftReceiptHeaderCount     %d\n", din->ftReceiptHeaderCount);
	for (unsigned int scnt = 0; scnt < din->ftReceiptHeaderCount; scnt++)
		wprintf(L"ftReceiptHeader      %d: %.*s\n", scnt, din->ftReceiptHeader[scnt].length, din->ftReceiptHeader[scnt].chars);

	wprintf(L"ftChargeItemsCount   %d\n", din->ftChargeItemsCount);

	wprintf(L"ftChargeLinesCount   %d\n", din->ftChargeLinesCount);
	for (unsigned int scnt = 0; scnt < din->ftChargeLinesCount; scnt++)
		wprintf(L"ftChargeLines        %d: %.*s\n", scnt, din->ftChargeLines[scnt].length, din->ftChargeLines[scnt].chars);

	wprintf(L"ftPayItemsCount      %d\n", din->ftPayItemsCount);

	wprintf(L"ftPayLinesCount      %d\n", din->ftPayLinesCount);
	for (unsigned int scnt = 0; scnt < din->ftPayLinesCount; scnt++)
		wprintf(L"ftPayLines           %d: %.*s\n", scnt, din->ftPayLines[scnt].length, din->ftPayLines[scnt].chars);

	wprintf(L"ftReceiptFooterCount %d\n", din->ftPayLinesCount);
	for (unsigned int scnt = 0; scnt < din->ftReceiptFooterCount; scnt++)
		wprintf(L"ftReceiptFooter     %d: %.*s\n", scnt, din->ftReceiptFooter[scnt].length, din->ftReceiptFooter[scnt].chars);

	wprintf(L"ftState:             %d  ftStateData: %.*s\n", (int)din->ftState, din->ftStateData.length, din->ftStateData.chars);

	wprintf(L"ftSignaturesCount    %d\n", din->ftSignaturesCount);
	for (unsigned int scnt = 0; scnt < din->ftSignaturesCount; scnt++)
	{
		wprintf(L"Format: %d Laenge %d: %.*s\n", 
			(int)din->ftSignatures[scnt]->ftSignatureFormat, 
			din->ftSignatures[scnt]->Data.length, 
			din->ftSignatures[scnt]->Data.length, 
			din->ftSignatures[scnt]->Data.chars);
	}
	wprintf(L"---------------------------------------------------------------------------\n");
#endif
	for (unsigned int scnt = 0; scnt < din->ftSignaturesCount; scnt++)
	{
		if (din->ftSignatures[scnt]->ftSignatureFormat == 3)
		{
			conv_wsstr2chr(din->ftSignatures[scnt]->Data, Signature);
			break;
		}
	}
}

void print_error()     //=========================================================================================================================
{
#ifndef Release_LIB
	ULONG errorCount;
	HRESULT hr = NOERROR;

	hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
	if (!FAILED(hr))
	{
		for (ULONG i = 0; i < errorCount; i++)
		{
			WS_STRING errorMessage;
			hr = WsGetErrorString(error, i, &errorMessage);
			if (!FAILED(hr))
			{
				wprintf(L"========================================================== ERROR ==========\n");
				wprintf(L"%.*s\n", errorMessage.length, errorMessage.chars);
			}
		}
	}
	else
	{
		wprintf(L"FATAL ERROR no information\n");
	}
	wprintf(L"========================================================== ERROR ==========\n");
#endif
}

extern "C" int deplib_exit() //=========================================================================================================================
{
	//Speicher aufräumen
	if (proxy)
	{
		WsCloseServiceProxy(proxy, NULL, NULL);
		WsFreeServiceProxy(proxy);
	}

//	if (heap)
//	{
//		WsFreeHeap(heap);
//	}

	if (error)
	{
		WsFreeError(error);
	}

	return 0;
}

