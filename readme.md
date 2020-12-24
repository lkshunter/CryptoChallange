# Crypto Challenge WS20/21

* Hochschule Emden Leer
* Studiengang: Informatik
* Vertiefungsrichtung: IT-Security
* Fach: Spezielle Verfahren der IT-Sicherheit
* Autor: Lukas Hagen

## Installation

Das Programm wurde in Clinon unter Windows geschrieben und ist für das Ausführen in dieser IDE gedacht. Dabei muss der
Pfad angepasst werden je nachdem welchen Compiler man verwendet. In unserem Projekt wurde es mit CygWin und dem
Windows-Subsystem für Linux (WSL) zum Laufen gebracht. Alle dafür benötigten Dateien liegen bei. Der Pfad für die
Dateien mit den Aufgaben muss unter Linux angepasst werden in der Methode:

````c++
vector<string> readChallengeIn(string filename) {
// Linux
// Pfad zum compilen unter Windows im WSL
ifstream t("/mnt/c/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);

// Windows
// Pfad zum compilen unter Windows mit CygWin
ifstream t("C:/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);
...
}
````

## Anwendungsbeispiele

### Angriffe mit dynamischem Public Key und Chitext einlesen

___

#### Fall 1: Eine der 14 Crypto Challenge Angriffe durchführen

In diesem Fall einfach die Methode *angriffGruppeN(int gruppe)* ausführen. Der Übergabe Parameter *gruppe* kann von 1
bis 14 gewählt werden.

````c++
int gruppenNummer = 1;
angriffGruppeN(gruppenNummer);
````

___

#### Fall 2: Einen eigenen Public Key und Chitext einlesen und brechen

Der Public Key und der Chitext werden über die Methode *angriffDatei(string file)* eingelesen und angegriffen. Übergeben
werden muss der Methode nur der Dateiname samt Endung. In diesem Fall muss die Datei ausgehend von der *main.cpp* im
Ordner "./Zusatsmaterialien/Aufgabe/" liegen.
(Solange der Pfad wie in Installation beschrieben nicht verändert wurde) Die Methode kann prinzipiell überall aufgerufen
werden. Zum Messen der Zeit ist es aber wichtig das es zwischen *begin* und *ende* liegt.

````c++
chrono::steady_clock::time_point begin = chrono::steady_clock::now();

angriffDatei("file.txt");

chrono::steady_clock::time_point ende = chrono::steady_clock::now();
````

Die Datei muss nach einem bestimmten Format formatiert sein:

````text
Public Key: [ x_1 * x_3 + x_2 * x_5 + x_9,
              x_3 + ...]
Chitext: [0,1,0,1,...]
````

Ein gutes Beispiel für den Aufbau einer solchen Datei sind die drei Angriffe d3, d5 und d7. Diese liegen als
.txt-Dateien aufbereitet auch im Ordner *Aufgabe*.

* BeispielAngriffd3.txt
* BeispielAngriffd5.txt
* BeispielAngriffd7.txt

___

### Angriffe "hardgecoded" (ohne einlesen)

Für den Fall das es wichtig ist zu testen, ob es performance technisch einen Unterschied macht ob der Key und der
Chitext dynamisch eingelesen wird hier eine .

````c++
void angriffHardCoded() {
    string publicKey = "x_1*x_3 + x_2*x_3 + x_2,\n"
                       "    x_1*x_3 + x_1 + x_2 + x_3,\n"
                       "    x_1*x_2 + x_3";

    vector<vector<vector<int>>> publicK = createPublicKey(publicKey);

    string clear = "0 1 1 0 0 0 1 0 1 1 0 0 0 0 1 0 1 0 1 1 0 1 1 1 1 0 1 0 1 1 1 0 0 0 1 0 1 0 1 0 1 1 1 1 1 0 0 0 0 0 1 0 1 0";

    vector<int> klartext;
    klartext = createVector(clear, ' ');

    vector<int> chi = {1, 1, 1};

    angriff(publicK, klartext, chi);
}
````

## Perfomance

___

Es lassen sich Unterscheide in der Performance feststellen je nachdem wie man den Code compilen lässt. Verglichen werden
die Varianten CygWin und das Windows-Subsystem für Linux.

| Angriff       | CygWin        | WSL           |
| :-----------: | ------------: | -------------:|
| d3            | 00:00:037:267 | 00:00:001:725 |
| d5            | 00:00:094:713 | 00:00:004:240 |
| d7            | 00:00:212:710 | 00:00:012:107 |
| Gruppe 1      | 02:21:830:675 | 01:10:507:878 |

Format der Zeitangabe [m:ss:ms:μm]

## Ablauf des Angriffs

### Public Key und Chitext aufbereiten

___
Der Public Key und der Chitext müssen von einem String jeweils in ein Format gebracht werden, das es einfacher macht,
mit ihnen zu arbeiten. Die Ausgangslage von Public Key und Chitext ist in **Fall 2: Einen eigenen Public Key und Chitext
einlesen und brechen** beschrieben.

#### Public Key

````c++
vector<vector<vector<int>>> createPublicKey(String pubKey)
````

Nach dem der String in seine Einzelteile zerlegt wurde, wird er nach folgender Logik in dem Vektor-Konstrukt
*vector\<vector\<vector\<int>>>* abgelegt.

````text
vector1_PublicKey(
    vector2_Funktion_1(
        vector3_multplikatio_1(
            1,2
        ),
	    vector3_multplikatio_2(
	 	    3,5
	    ),
	    vector3_multplikatio_3(
	 	    3
	    ),
	    vector3_multplikatio_4(
	 	    4
	    ),
	    ...
	    vector3_multplikatio_n(
	        ...
	    )
	),
	...
	vector2_Funktion_n(
	    ...
	)
)
````

Den äußersten Vector kann als eine Funktion des Public Key verstanden werden. Die darin enthaltenen Vektoren sind die
Darstellungen der einzelnen Summanden einer Funktion. Und die darin enthaltenen sind die Multiplikanden der Funktion.

#### Chitext

````c++
vector<int> createVector(String chi, char seperator)
````

Im Chitext soll jedes Bit einzeln im, einem *int* Vektor abgelegt werden. Dazu wird der String an jedem Komma
aufgetrennt und die Ziffern mit *std::stoi()* in einen *int* gecastet. Danach wird der *int* in den Vektor gespeichert.

### Erzeugen des Klar-Geheim Kompromisses

___
Um den Klar-Geheim Kompromiss (KGK) zu erzeugen, muss der Chitext (0,0,1) mit dem Klartext (0, 1, 1) multipliziert
werden. Es muss eine gewisse Menge von diesem KGK erzeugt werden in der Vorlesung, wurde 2*n^2 empfohlen. Bei dem
Beispiel d3 sind das 18 Zeilen à 9 Bits. Bei eigenen Versuchen die Laufzeit zu optimieren wurde der KGK drastisch
verringert. Hier ist aber anzumerken, das irgendwann nicht mehr genug Informationen vorliegen, um die Verschlüsselung zu
brechen.

````text
Chi     Klar    Klar-Geheim Kompromiss
          0     0 0 0
0 0 1  *  1  =  0 0 1  -->  0 0 0 0 0 1 0 0 1
          1     0 0 1
````

Der Ablauf und die verschiedenen Schritte sind in der Methode beschrieben. In dieser Implementation wird nur der
einfache Gauß angewendet. In der Vorlesung wurde der *reduced row echolon* empfolen. Diese Form des Gauß Algorithmus
entfernt auch über der Stufenform möglichst viel Nullen.

````c++
NTL::mat_GF2 createTriangleMatrix(vector<int> &clear, vector<int> &chi, int l) {
    NTL::mat_GF2 triangle;
    triangle.SetDims((clear.size() / l), l * l);
    NTL::clear(triangle);
    int m, n = 0;

    /*
     * Definieren der benötigten Matrizen
     * */
    NTL::mat_GF2 vec; // Klartext Spalten: 1, Zeilen: l
    NTL::mat_GF2 mat; // Chitext Splaten: l, Zeilen: 1
    NTL::mat_GF2 erg; // Ergebniss der Multplikation Splaten: l, Zeilen: l
    vec.SetDims(l, 1);
    mat.SetDims(1, l);

    /*
     * Jede Zeile des Klartextes wird mit dem dazugehörigen Chitext multipliziert
     * */
    for (int i = 0; i < clear.size(); i += l) {

        // Leeren der Matrizen
        NTL::clear(vec);
        NTL::clear(mat);
        NTL::clear(erg);

        // Füllen der Matrizen
        for (int j = i; j < (i + l); j++) {
            vec[j - i][0] = clear.at(j);
            mat[0][j - i] = chi.at(j);
        }

        /*
         * Multiplikation von Klartext und Chitext
         * */
        NTL::mul(erg, vec, mat);
        
        
        m = 0;
        for (int a = 0; a < erg.NumRows(); a++) {
            for (int b = 0; b < erg.NumCols(); b++) {
                triangle[n][m] = erg.get(a, b);
                m++;
            }
        }
        n++;
    }

    cout << triangle << "\n" << endl;
    
    // Anwendung des Gauss Algorithmus durch NTL
    NTL::gauss(triangle);

    cout << triangle << "\n" << endl;

    return triangle;
}
````

### Aufrollen der Matrix

___
todo

### Erstellen der speziellen Lösung und daraus resultierenden Basis

___
todo

### Chitext entschlüsseln und überprüfen

___
todo

## Bibliotheken

___
Es wurde die mathematische Bibliothek NTL (*NTL: A Library for doing Number Theory*) verwendet.<br>
Weitere Informationen hierzu sind unter diesem *[Link](https://libntl.org/)* zu finden.


