#C Library fuer die Anbindung an den fiskaltrust dienst

###Version 0.2 für fiskaltrustdienst vom 30.6.2016

##Projektidee
- Einfach C Anbindung an den Fiskaltrustdienst
- Erstellen einer möglichst einfach verwendbaren C-Schnittstelle für die Fiskaltrust SOAP Schnittstelle
- Einfaches C-Testprogramm zum Testen der erstellten Library

Die Fiskaltrust SOAP Schnittstelle soll mit einer Ingres 4GL Datenbankanwendung verwendet werden. Die 4GL Anwendung kann nur mit VC10.0 kompiliert werden.
Die Fiskaltrust C Schnittstelle kann nur mit VC 14.0 (Visual Studio 2015) kompiliert werden, nun muss eine Schnittstelle zwischen diesen beiden Versionen geschaffen werden.
Hierzu wird mit Visual Studio 15.0 eine C-Library erstellt, welche in die VC10.0 Umgebung eingebunden werden kann, damit ist es möglich die Funktionen der Library mit der Ingres 4GL Anwendung zu verwenden. Diese C-Funktionen sollen möglichst einfach Aufrufe anbieten, und eine Testumgebung für die SOAP Schnittstelle.
Komplizierte Funktions- und Datenstrukturen können von der 4GL Anwendung nicht aufgerufen werden.

Die veröffentlichen Sourcen sollen für C-Entwickler einen einfachen Einstieg in die Anbindung an die SOAP Schnittstelle bieten. Keine Haftung für Fehler usw.

Grundlage für dieses Projekt sind die Projekte auf Github: 
fiskaltrust/demo und fiskaltrust/interface


##Testsystem
- PC mit Windows 7 prof. 64 Bit
- Visual C 10.0 (32 Bit)
- Visual Studio Express 2015 für Windows Desktop
- Ingres 10.0 (32 Bit)

##Veröffentliche Dateien
| Datei | Beschreibung |
| --- | --- |
|deplib.cpp | C/C++ Source |
|deplip.h | Headerdatei |
|deplib_test.sln | VS2015 Solution |
|deplib_test.vcxproj | VS2015 VC++ Project |
|deplib_test.vcxproj.filters | VS2015 VC++ Project | 
|deplib_test.vcxproj.user | VS2015 VC++ Project |
|Release_LIB/... | erstellte C-Library |
|fiskaltrust.interface.wsdl.c | erstellt mit ws_erstellen.bat |
|fiskaltrust.interface.wsdl.h | erstellt mit ws_erstellen.bat |
|stdafx.cpp | übernommen von fiskaltrust |
|stdfx.h | übernommen von fiskaltrust |
|targetver.h | übernommen von fiskaltrust |
|tools/… | übernommen von fiskaltrust |
|test_lib.c | C-Source zum Testen der deplib |
|test_lib.exe | Testprogramm |
|test_lib_make.bat | Batch zum Erstellen des Testprogramms |

##Verwenden der Testumgebung
1. Downloaden des Gesamten Projekts von Github in z.B.: \deplib
2. Starten des Fiskaltrust Dienstes:
  \deplib\tools\fiskaltrust-net40\test.cmd
3. Visual Studio 2015 Projekt öffnen
  \deplib\deplib_test.sln
4. Das Projekt kann mit 3 verschiedenen Konfigurationen erstellt werden:
  • Release_LIB…es wir die Library Release_LIB/deplib_test1.lib erstellt
  • Debug………...zum Testen, die Funktion main() wird mit compiliert
  • Release.……...zum Testen, die Funktion main() wird mit compiliert
5. Testprogramm test_lib
  • Übersetzen des Programms test_lib:
    Im Explorer doppelklicken auf \deplib\test_lib_make.bat
  • Starten des Programms:
    Im Explorer doppelklicken auf \deplib\test_lib.exe

##Offene Punkte
  • DEP auslesen funktioniert nur bis zu einer Länge von 65536
  • Summenzähler auslesen
  • Monatsbeleg/Jahresbeleg
  • Statusmeldungen auslesen

##Datenübergabe
In diesen Beispielen werden nur Rechnungssummen und keine einzelne Artikel an die Schnittstelle übergeben. Grundlage für diese Vorgehensweise:

Auszug aus diversen FAQs an das BMF:

Wenn eine Rechnung ausgestellt wird, unabhängig ob mit Excel bzw. Word (wird dann der Buchhaltung übergeben) oder über ein ERP/WWS, und diese bereits zur Abfuhr der Steuerschuld herangezogen wird, dann ist bei Erfassung in der Registrierkasse hier ein Verweis auf die Rechnung und keine Aufschlüsselung der Steuersätze zulässig und ausreichend. Ist das korrekt? Oder muss die Rechnung im selben System wie der Registrierkasse erfasst werden?
Ja, der Betrag gehört dann auch dem Null% Umsatz zugeordnet (Bruttobetrag).

"Mit Hilfe der Registrierkasse wird ein Zahlungsbeleg (keine Rechnung!) ausgestellt der lediglich auf die Rechnung verweist und keine Aufschlüsselung nach Umsatzsteuersätzen enthält. Siehe Punkt 2.4.11: „Es ist zulässig auf diesem Beleg lediglich auf die Nummer der Rechnung zu verweisen und keine Aufschlüsselung der Umsätze nach Steuersätzen vorzunehmen, wenn die Rechnung zur Abfuhr der Steuerschuld schon im (elektronischen) Aufzeichnungssystem erfasst wurde.“ 
c) Der Beleg verweist auf die Rechnung, enthält keine Aufschlüsselung nach Umsatzsteuersätzen und somit auch nicht im DEP. 
d) Dieser Zahlungsbetrag ist im DEP der Registrierkasse vollständig als „Betrag-Satz-Null“ zu erfassen. Stimmt und gehört auch in den Umsatzzähler. 
e) Als (elektronisches) Aufzeichnungssystem ist auch ein klassisches Hotelprogramm zu verstehen, welches Rechnung in fortlaufender Nummerierung ausstellt (ähnlich der Funktionalität eines einfachen Warenwirtschaftssystems). 

