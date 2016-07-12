/* 
2016-06-14
Testprgramm fÅr fiskaltrust Dummy Dienst
verwendet die Library cppConsoleApplicationSOAP-Test1.lib
Kann mit VC 2010 oder VC 2015 compiliert werden
Keine Haftung
Copyright H.Teufelhart 
*/

#include <stdio.h>
#include "deplib.h"

char buf_sign[XMAS_MAX_CHAR];

int main()
{
	int ret;
	
	printf("Vor deplib_init()\n");
	ret=deplib_init();
	printf("deplib_init()             ret:%d\n", ret);

	ret=deplib_send_hello("Hello world");
	printf("deplib_send_hello()       ret:%d\n", ret);

	ret = deplib_sign_startbeleg(buf_sign);
	printf("deplib_sign_startbeleg()  ret:%d  Signatur: %s\n", ret, buf_sign);

	ret=deplib_sign("Rechnung 123", "Bar", 123.45, 0.0, buf_sign);
	printf("deplib_sign()             ret:%d  Signatur: %s\n", ret, buf_sign);

	ret=deplib_sign_null(buf_sign);
	printf("deplib_sign_null()        ret:%d  Signatur: %s\n", ret, buf_sign);

	{
	int len;
	char *dep;
	ret = deplib_get_dep(&len, &dep);
	printf("-------- RSKV DEP ---- Start --------------------------------------------------\n");
	printf("%.*s\n", len, dep);
	printf("-------- RSKV DEP ---- Ende ---------------------------------------------------\n");
	printf("deplib_get_dep()          ret:%d  Lenght: %d\n\n", ret, len);
	}

	ret=deplib_exit();
	printf("deplib_exit()             ret:%d\n", ret);
	
	printf("Ende: irgendeine Taste druecken zum beenden");
	ret = getchar();

}
