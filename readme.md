# Crypto Challenge WS20/21

* Hochschule Emden Leer
* Studiengang: Informatik
* Vertiefungsrichtung: IT-Security
* Fach: Spezielle Verfahren der IT-Sicherheit
* Autor: Lukas Hagen

## Installation

Das Programm wurde in Clion unter Windows geschrieben und ist für das Ausführen in dieser IDE gedacht. Dabei muss der
Pfad angepasst werden je nachdem welchen Compiler man verwendet. Unser Projekt wurde mit CygWin und dem
Windows-Subsystem für Linux \(WSL\) zum Laufen gebracht. Alle dafür benötigten Dateien liegen bei. Der Pfad für die
Dateien mit den Aufgaben muss unter Linux angepasst werden in der Methode:

````c++
int main() {
srand(time(0));

// Windows-Subsystem für Linux
glb_path = "/mnt/c/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/";
// Windows
glb_path = "C:/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/";

time_t now = time(0);
...
}
````

## Anwendungsbeispiele

### Angriffe mit dynamischem Public Key und Chitext einlesen

___

#### Fall 1: Einen der 14 Crypto Challenge Angriffe durchführen

In diesem Fall einfach die Methode `angriffGruppeN(int gruppe)` ausführen. Der Übergabeparameter `gruppe` kann von 1
bis 14 gewählt werden. Der Parameter `mode` gibt an welcher Modus verwendet werden soll. Die `0` enstspricht dem
normalen Modus hier werden `2\*n\^2` Klar-Geheim-Kompromisse verwendet. Um die Laufzeit bei größeren Public Keys zu
verringern, gibt es den **Performance Mode**. Genaueres dazu wird im Abschnitt Performance erläutert.

````c++
angriffGruppeN(int gruppenNummer, int mode);
````

___

#### Fall 2: Einen eigenen Public Key und Chitext einlesen und brechen

Der Public Key und der Chitext werden über die Methode `angriffDatei(string file)` eingelesen und angegriffen. Übergeben
werden muss der Methode nur der Dateiname samt Endung. In diesem Fall muss die Datei ausgehend von der `main.cpp` im
Ordner `"./Zusatsmaterialien/Aufgabe/"` liegen.
(Solange der Pfad wie in Installation beschrieben nicht verändert wurde) Die Methode kann prinzipiell überall aufgerufen
werden. Zum Messen der Zeit ist es aber wichtig, dass es zwischen `begin` und `ende` liegt.

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
.txt-Dateien aufbereitet auch im Ordner `Aufgabe`.

* BeispielAngriffd3.txt
* BeispielAngriffd5.txt
* BeispielAngriffd7.txt

___

### Angriffe "hardgecoded" (ohne einlesen)

Für den Fall das es wichtig ist zu testen, ob es performance technisch einen Unterschied macht ob der Key und der
Chitext dynamisch eingelesen wird hier eine.

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

### Unterschiede in der Art der Toolchain
___
Es lassen sich Unterscheide in der Performance feststellen je nachdem wie man den Code compilen lässt. Verglichen werden
die Varianten CygWin und das Windows-Subsystem für Linux.

| Angriff       | CygWin        | WSL           |
| :-----------: | ------------- | -------------:|
| d3            | 00:00:037:267 | 00:00:001:725 |
| d5            | 00:00:094:713 | 00:00:004:240 |
| d7            | 00:00:212:710 | 00:00:012:107 |
| Gruppe 1      | 02:21:830:675 | 01:10:507:878 |

Format der Zeitangabe `[m:ss:ms:μm]`

### Der Performane Mode

___

Da sich bei größeren Public Keys die Laufzeit drastisch erhöt, gibt es den **Performance Mode**. Hier werden die
erzeugten Bits für den Klar-Geheim Kompromiss reduziert.

## Ablauf des Angriffs

### Public Key und Chitext aufbereiten

___
Der Public Key und der Chitext müssen von einem String jeweils in ein Format gebracht werden, das es einfacher macht,
mit ihnen zu arbeiten. Die Ausgangslage von Public Key und Chitext ist
in `Fall 2: Einen eigenen Public Key und Chitext einlesen und brechen` beschrieben.

#### Public Key

````c++
vector<vector<vector<int>>> createPublicKey(String pubKey)
````

Nachdem der String in seine Einzelteile zerlegt wurde, wird er nach folgender Logik in dem Vektor-Konstrukt
`vector\<vector\<vector\<int>>>` abgelegt.

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

Der äußerste Vector kann als eine Funktion des Public Key verstanden werden. Die darin enthaltenen Vektoren sind die
Darstellungen der einzelnen Summanden einer Funktion. Und die darin enthaltenen sind die Multiplikanden der Funktion.

#### Chitext

````c++
vector<int> createVector(String chi, char seperator)
````

Im Chitext soll jedes Bit einzeln in einem `int` Vektor abgelegt werden. Dazu wird der String an jedem Komma
aufgetrennt und die Ziffern mit `std::stoi()` in einen `int` gecastet. Danach wird der `int` in den Vektor gespeichert.

### Erzeugen des Klar-Geheim Kompromisses

___
Um den Klar-Geheim Kompromiss (KGK) zu erzeugen, muss der Chitext `(0,0,1)` mit dem Klartext `(0, 1, 1)` multipliziert
werden. Es muss eine gewisse Menge von diesen KGK erzeugt werden. In der Vorlesung wurden `2*n^2` KGK empfohlen. Bei dem
Beispiel d3 sind das 18 Zeilen à 9 Bits. Bei eigenen Versuchen die Laufzeit zu optimieren wurde der KGK drastisch
verringert. Dafür wurde der Performance Modus implementiert. Dieser Modus ist zwar schneller, kann aber unter Umständen
kein Ergebnis liefern.

````text
Chi     Klar    Klar-Geheim Kompromiss
          0     0 0 0
0 0 1  *  1  =  0 0 1  -->  0 0 0 0 0 1 0 0 1
          1     0 0 1
````

Der Ablauf und die verschiedenen Schritte sind in der Methode beschrieben. In dieser Implementation wird nur der
einfache Gauß angewendet. In der Vorlesung wurde der *reduced row echolon* empfohlen. Diese Form des Gauß Algorithmus
entfernt auch über der Stufenform möglichst viele Einsen.

````c++
NTL::mat_GF2 createTriangleMatrix(vector<int> &clear, vector<int> &chi, int l) {
    NTL::mat_GF2 triangle;
    triangle.SetDims((clear.size() / l), l * l);
    int m, n = 0;
    
    NTL::mat_GF2 vec;
    NTL::mat_GF2 mat;
    NTL::mat_GF2 erg;
    vec.SetDims(l, 1);
    mat.SetDims(1, l);
    
    for (int i = 0; i < clear.size(); i += l) {
        
        NTL::clear(vec);
        NTL::clear(mat);
        NTL::clear(erg);
        
        for (int j = i; j < (i + l); j++) {
            vec[j - i][0] = clear.at(j);
            mat[0][j - i] = chi.at(j);
        }
        
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
    
    NTL::gauss(triangle);
    
    cout << triangle << "\n" << endl;

    return triangle;
}
````

Dieser Teil des Codes ist bei größernen Key's und entsprechendem KGK sehr rechenintensiv.</br>
(Todo: Ablauf irgendwie vereinfachen)

### Erkennen und speichern der freien Variablen

___
Um die freien Variablen zu ermitteln werden in unserer Implementation erst alle führenden Einsen pro Zeile gespeichert.
Das Schleifenkonstrukt geht aber nicht jeden Wert in der Matrix durch. Zur Verbesserung der Laufzeit gibt es
Abbruchkriterien. Wie im Code zu sehen ist, wenn das erste if-Statement erfüllt ist, wurde die erste Eins gefunden. Um
den Schleifendurchlauf zu beenden wird `n` auf den maximalen Wert gesetzt. Gleichzeitig wird `start` auf `n+1`
gesetzt das bewirkt, dass im nächsten Schritt die Schleife nicht bei null anfängt, sondern an der Stelle wo in einer
idealen Matrix die nächte Eins auf der Zeilenstufe wäre. So können viele Iterationsschritte vermieden werden. Da
unterhalb der Zeilen-Stufenform keine Einsen existieren. Außerdem wird die Position der Eins auf der Zeilenstufe
gespeichert.

```c++
for (int i = 0; i < gaussMatrix.NumRows(); i++) {
    int start = 0;
    for (int n = start; n < gaussMatrix.NumCols(); n++) {
        if (!(NTL::IsZero(gaussMatrix[i][n]))) {
            oneSteps.push_back(vector<int>{i, n});
            start = n + 1;
            n = gaussMatrix.NumCols();
        }
        if (n == (gaussMatrix.NumCols() - 1)) {
            rowMax = i - 1;
            i = gaussMatrix.NumRows();
        }
    }
}
```

### Aufrollen der Matrix

___
Für diesen Schritt und noch folgende Schritte müssen Daten erfasst und gespeichert werden. Ausschließlich für das
Aufrollen wird der Vektor `expectedOne` verwendet. Hier werden an den Stellen 0,1 die Zeilen und Spalten Nummer der
nächsten erwarteten Zeilen-Stufen Eins abgespeichert. Die erste erwartete Eins liegt in der unteren rechten Ecke der
Matrix. Im Laufe des Programmes werden diese Werte für jede kommende Zeile angepasst. Wichtig für das Erzeugen der
späteren speziellen Lösungen sind die Vektoren `freeVar` und `rowJump`. In `freeVar` werden, wie der Name vermuten lässt,
die freien Variablen gespeichert. `rowJump` wird verwendet um zu speichern in welchen Zeilen mehr als eine freie
Variable gefunden wurde.</br>

```c++
if (!(NTL::IsZero(gaussMatrix))) {
    vector<int> expectedOne;
    vector<int> freeVar;
    vector<int> rowJump;

    expectedOne.push_back(rowMax);
    expectedOne.push_back(gaussMatrix.NumCols() - 1);

    for (int i = rowMax; i >= 0; i--) {
        if (!(expectedOne.at(0) == oneSteps.at(i).at(0) && expectedOne.at(1) == oneSteps.at(i).at(1))) {
            for (int n = expectedOne.at(1); n > oneSteps.at(i).at(1); n--) {
                rowJump.push_back(i);
                freeVar.push_back(n);
            }

            if (i > 0) {
                expectedOne.at(0) = oneSteps.at(i).at(0);
                expectedOne.at(1) = oneSteps.at(i).at(1);
            }
        }

        expectedOne.at(0) -= 1;
        expectedOne.at(1) -= 1;
    }
    ...
}
```

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


