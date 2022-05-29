# Crypto Challenge WS20/21

* Hochschule Emden Leer
* Studiengang: Informatik
* Vertiefungsrichtung: IT-Security
* Fach: Spezielle Verfahren der IT-Sicherheit

## Installation

Das Programm wurde in Clion unter Windows geschrieben und ist für das Ausführen in dieser IDE gedacht. Dabei muss der
Pfad angepasst werden je nachdem welchen Compiler man verwendet. In unserem Projekt wurde es mit CygWin und dem
Windows-Subsystem für Linux \(WSL\) zum Laufen gebracht. Alle dafür benötigten Dateien liegen bei. Der Pfad für die
Dateien mit den Aufgaben muss unter Linux angepasst werden in der Methode:

````c++
int main() {
srand(time(0));

// Windows-Subsystem für Linux
glb_path = "/mnt/c/Users/Lukas/CLionProjects/CryptoChallenge/Zusatzmaterialien/Aufgabe/";
// Windows
glb_path = "C:/Users/Lukas/CLionProjects/CryptoChallenge/Zusatzmaterialien/Aufgabe/";

time_t now = time(0);
...
}
````

## Anwendungsbeispiele

### Command Line Interface - *CLI*

___

#### Nutzung

````c++
int main() {
    ...
    cliUserInterface();
    ...
}
````

Beim Start der CLI werden dem Nutzer 3 Fragen gestellt ob Features eingeschaltet werden sollen:

* Erweiterte Ausgabe
* Multithreading
* Performance Modus Diese können mit *y* bestätigt oder mit *n* verneint werden. Danach kann ein regulärer Befehl
  eingegeben werden.

#### Befehlsliste

Für die Bedienung des Command Line Interfaces sind verschiedenen Befehle implementiert worden.</br>

* gruppe
  * Eine Gruppennummer im Bereich von einschließlich 1 bis 14 wird als Eingabe verlangt
* angriff
  * Eine Anriffsnummer wird verlangt (3,5,7)
* datei
  * Es muss eing Dateiname samt Endung eingegeben werden (z.B. Datei.txt)
* multithreading
  * Ändert die Einstellung ob Multithreading aktiviert ist
* performance
  * Ändert die Einstellung ob Performance Modus aktiviert ist
* ausgabe
  * Ändert die Einstellung ob Performance Modus aktiviert ist
* status
  * Gibt die aktuellen Einstellungen aus
* exit
  * Schließt die Anwendung

### Angriffe mit dynamischem Public Key und Chitext einlesen

___

#### Fall 1: Eine der 14 Crypto Challenge Angriffe durchführen

In diesem Fall einfach die Methode `angriffGruppeN(int gruppe)` ausführen. Der Übergabe Parameter `gruppe` kann von 1
bis 14 gewählt werden. Der Parameter `mode` gibt an welcher Modus verwendet werden soll. Die `0` entspricht dem Normalen
Modus hier wird für den Klar-Geheim Kompromiss `2*n^2` verwendet. Um die Laufzeit bei größeren Public Keys zu
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
werden. Zum Messen der Zeit ist es aber wichtig das es zwischen `begin` und `ende` liegt.

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

### ~~Angriffe "hardgecoded" (ohne einlesen)~~

*(seit der Implementation des Multithreading nicht mehr funktionsfähig)*</br></br>

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

vector<int> chi = { 1, 1, 1 };

angriff(publicK, klartext, chi);
}
````

## Performance

### Multithreading

___
Multithreading wird in dem Programmablauf an 2 Stellen genutzt, wenn es aktiviert ist. Es findet bei der Verschlüsseln
von Klartext und dem Erzeugen von speziellen Lösungen zu einer freien Variable statt. Es werden standard mäßig nur 4
Threads genutzt. Eine Methode, die sich dynamisch an die Gegebenheiten anpasst, ist in arbeit.

### Der Performance Modus

___
Der Performance Modus kann die Laufzeit bei großen Schlüsseln drastisch verkürzen. Das Problem besteht in seiner Idee.
Der Modus erzeugt nu die Hälfte des in der Vorlesung vorgeschriebenen Klar-Geheim Kompromisses. Wenn in diesem KGK sich
zu viele Zeilen doppel oder zu viele Null Zeilen entstehen kann es sein das nicht genug Informationen für einen
erfolgreichen Angriff vorliegen. Unabhängig vom Performance Modus kann dies auch bei sehr kleinen Schlüsseln auftreten.

### Zeitmessungen

___
Es lassen sich Unterscheide in der Performance feststellen je nachdem wie man den Code compilen lässt. Verglichen werden
die Varianten CygWin und das Windows-Subsystem für Linux.

#### Werte eines Zwischenstandes der Software ohne Multithreading (alle Werte im Performance Modus gemessen)

| Angriff       | CygWin        | WSL           |
| :-----------: | ------------- | -------------:|
| d3            | 00:00:037:267 | 00:00:001:725 |
| d5            | 00:00:094:713 | 00:00:004:240 |
| d7            | 00:00:212:710 | 00:00:012:107 |
| Gruppe 1      | 02:21:830:675 | 01:10:507:878 |

#### Werte mit Multithreading

Der Code wurde nicht im Debugging Modus, sonder im Release Modus kompiliert.

| Angriff                    | CygWin        | WSL           |
| -------------------------- | ------------: | ------------: |
| d3                         | 00:00:012:415 | 00:00:001:124 |
| d3 Performance Modus       | 00:00:015:269 | 00:00:001:235 |
| d5                         | 00:00:015:696 | 00:00:001:811 |
| d5 Performance Modus       | 00:00:016:153 | 00:00:001:649 |
| d7                         | 00:00:029:544 | 00:00:003:137 |
| d7 Performance Modus       | 00:00:016:376 | 00:00:002:987 |
| Gruppe 1                   | 00:02:745:677 | 00:10:371:602 |
| Gruppe 1 Performance Modus | 00:01:748:339 | 00:07:186:299 |

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

Nach dem der String in seine Einzelteile zerlegt wurde, wird er nach folgender Logik in dem Vektor-Konstrukt
`vector\<vector\<vector\<int>>>` abgelegt.

````text
vector1_PublicKey(
    vector2_Funktion_1(
        vector3_multiplication_1(
            1,2
        ),
	    vector3_multiplication_2(
	 	    3,5
	    ),
	    vector3_multiplication_3(
	 	    3
	    ),
	    vector3_multiplication_4(
	 	    4
	    ),
	    ...
	    vector3_multiplication_n(
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

#### Abgefangenen Chitext aufbereiten

````c++
vector<int> createVector(String chi, char seperator)
````

Im Chitext soll jedes Bit einzeln im, einem `int` Vektor abgelegt werden. Dazu wird der String an jedem Komma
aufgetrennt und die Ziffern mit `std::stoi()` in einen `int` gecastet. Danach wird der `int` in den Vektor gespeichert.

### Verschlüsseln für den selbst erzeugten Klar-Geheim Kompromisses

___
Dies ist eine der zwei Stellen an der in dem Programm Multithreading implementiert wurde.

#### Singlethread

Im Singlethread wird der Klartext Vector `clear` immer um die Länge des Public Key weiter iteriert. Das heißt, hat der
Public Key eine Länge von 49 Funktionen Spring die Schleife immer um diese Anzahl weiter. Die nächste Schleife geht dann
den Public Key Formel für Formel durch und errechnet jeweils ein Bit des Chitextes.

````c++
for (int o = 0; o < clear.size(); o += pKey.size()) {
    
  for (int i = 0; i < pKey.size(); i++) {
      cipher = 0;
      
      for (int j = 0; j < pKey.at(i).size(); j++) {
          vector<int> teilFunktion = pKey.at(i).at(j);
          
          if (teilFunktion.size() == 2) {
              cipher += clear.at(o + teilFunktion.at(0) - 1) * clear.at(o + teilFunktion.at(1) - 1);
          } else if (teilFunktion.size() == 1) {
              cipher += clear.at(o + teilFunktion.at(0) - 1);
          }
      }

      cipher = cipher % 2;
      chi.push_back(cipher);
  }
  ...
}
````

#### Multithreading

Im Multithreading geschieht das gleiche wie im Singlethread, nur wird der Klartext Vektor hier auf 4 Threads aufgeteilt.
Kann die benötigte Zeit für das Verschlüsseln verkürzt werden. Die Threads selbst sind als Lambda Funktion definiert. Im
Gegensatz zur Singlethread Funktion bekommen sie einen `start` und einen `end` Index übergeben. Nur in diesem Bereich
des `clear`
Vektors verschlüsselt jeder Thread. Am Ende der Funktion wird über `task.join()` darauf gewartet.

```c++
    double tmp = ((clear.size() / (double) pKey.size()) / 4.0);
    int range = ceil(tmp);
    int diff = (range * 4) - (clear.size() / pKey.size());
    int diff1 = 0;
    int diff2 = 0;
    int diff3 = 0;

    vector<vector<int>> results;
    vector<int> result;
    results.push_back(result);
    results.push_back(result);
    results.push_back(result);
    results.push_back(result);

    if ((diff - range) > 0) {
        diff1 = diff - range;

        if ((diff1 - diff) > 0) {
            diff1 = range;
            diff2 = diff - range;

            if ((diff2 - diff) > 0) {
                diff1 = range;
                diff2 = range;
                diff3 = diff - range;
            }
        }
    }

    thread task1(encyptDistributed, 1, std::ref(clear), 0, range - diff3, std::ref(results.at(0)), std::ref(pKey));
    thread task2(encyptDistributed, 2, std::ref(clear), range, (range * 2) - diff2, std::ref(results.at(1)), std::ref(pKey));
    thread task3(encyptDistributed, 3, std::ref(clear), range * 2, (range * 3) - diff1, std::ref(results.at(2)), std::ref(pKey));
    thread task4(encyptDistributed, 4, std::ref(clear), range * 3, ((range * 4) - diff), std::ref(results.at(3)), std::ref(pKey));

    task4.join();
    task1.join();
    task2.join();
    task3.join();

```

### Erzeugen des Klar-Geheim Kompromisses

___
Um den Klar-Geheim Kompromiss (~~~~KGK) zu erzeugen, muss der Chitext `(0,0,1)` mit dem Klartext `(0, 1, 1)`
multipliziert werden. Es muss eine gewisse Menge von diesem KGK erzeugt werden in der Vorlesung, wurde `2*n^2`
empfohlen. Bei dem Beispiel d3 sind das 18 Zeilen à 9 Bits. Bei eigenen Versuchen die Laufzeit zu optimieren wurde der
KGK drastisch verringert. Dafür wurde der Performance Modus implementiert. Dieser Modus ist zwar schneller kann aber
unter Umständen kein Ergebnis liefern.

````text
Chi     Klar    Klar-Geheim Kompromiss
          0     0 0 0
0 0 1  *  1  =  0 0 1  -->  0 0 0 0 0 1 0 0 1
          1     0 0 1
````

Der Ablauf und die verschiedenen Schritte sind in der Methode beschrieben. In dieser Implementation wird nur der
einfache Gauß angewendet. In der Vorlesung wurde der *reduced row echolon* empfohlen. Diese Form des Gauß Algorithmus
entfernt auch über der Stufenform möglichst viel Nullen.

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

Dieser Teil des Codes ist bei größeren Key's und entsprechendem KGK sehr rechen intensiv.</br>
(Todo: Ablauf irgendwie vereinfachen)

### Erkennen und speichern der freien Variablen

___
Um die freien Variablen zu ermitteln werden in unserer Implementation erst alle führenden Einsen pro Zeile gespeichert.
Das Schleifen Konstrukt geht aber nicht jeden Vert in der Matrix durch. Zur Verbesserung der Laufzeit gibt es
Abbruchkriterien. Wie im Code zu sehen ist wenn, das erste if-Statement erfüllt ist, wurde die erste Eins gefunden. Um
den Schleifendurchlauf zu beenden wird `n` auf den maximalen Wert gesetzt. Gleichzeitig wird `start` auf `n+1`
gesetzt das bewirkt, dass im nächsten Schritt die Schleife nicht bei null anfängt, sondern an der Stelle wo in einer
idealen Matrix die nächte Eins auch der Zeilenstufe wäre. So können viele Iterations schritte vermieden werden. Da
unterhalb der Zeilen-Stufenform keine einsen existieren. Außerdem wird die Position der Zeilen-Stufenform Eins
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
späteren speziellen Lösungen sind die Vektoren `freeVar` und `rowJump`. In `freeVar` werden, wie der Name vermuten lässt
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

#### Erzeugen der Matrizen aus den speziellen Lösungen

Aus den speziellen Lösungen werden Matrizen erzeugt die für die Generierung der "xy-Formel" benötigt werden.</br>
Beispiel:

```text
Spezielle Lösungen des Beispielangriffes d3:
   [[1 0 0 0 0 1 1 1 1]
    [0 0 1 1 1 0 1 0 0]]

Sich ergebende Matrizen:
     x_1   |   x_2   |   x_3
   ------- | ------- | -------
    1 0 0  |  0 0 1  |  1 1 1
    0 0 1  |  1 1 0  |  1 0 0
```

#### Erzeugen der xy-Formel

todo</br>

```text
Sich ergebende Matrizen:
     x_1   |   x_2   |   x_3
   ------- | ------- | -------
    1 0 0  |  0 0 1  |  1 1 1
    0 0 1  |  1 1 0  |  1 0 0
    
Sich ergebende Formeln aus Zeile eins aller Matrizen:
x_1 * y_1 + x_2 * y_3 + x_3 * y_1 + x_3 * y_2 + x_3 * y_3

Sich ergebende Formeln aus Zeile zwei aller Matrizen:
x_1 * y_3 + x_2 * y_1 + x_2 * y_2 + x_3 * y_1

```

Der untenstehende Code zeigt wie die Formeln aus dem obrigen Beispiel in der Gleichen Art gespeichert werden wie der
Public Key.

````c++
vector<vector<vector<int>>> xyFormeln;
for (int i = 0; i < specialSolution.size(); i++) {
    vector<vector<int>> tmp2;
    for (int n = 0; n < matrizen.size(); n++) {
        for (int o = 0; o < breite; o++) {
            if (!(NTL::IsZero((matrizen.at(n))[i][o]))) {
                vector<int> tmp;
                tmp.push_back(n + 1);
                tmp.push_back(o);
                tmp2.push_back(tmp);
            }
        }
    }
    xyFormeln.push_back(tmp2);
}
````

#### xy-Formel kürzen

Um die Formel zu kürzen, muss für jeden y-Wert der Wert an der Stelle im Chitext eingesetzt werden bei einer
Multiplikation mit 0 fällt der x-Wert automaitisch weg. Im nächsten Schritt wird überprüft wie oft ein Wert noch in der
Formel auftritt. Ist die Anzahl modulo 2 gleich 0 so müssen auch diese in der gekürzten Form nicht mehr berücksichtigt
werden.

```text
Formeln die sich durch die sich durch die Matrizen ergeben.
(x_1 * y_1 + x_2 * y_3 + x_3 * y_1 + x_3 * y_2 + x_3 * y_3)
(x_1 * y_3 + x_2 * y_1 + x_2 * y_2 + x_3 * y_1)

Abgefangener Chitext:
(1 1 1)

Gekürzte Form:
x_1 + x_2 + x_3
x_1 + x_3
```

Der erste for-Schleifen Block multipliziert den x-Wert mit y-Stelle im Chitext. Der zweite Block, der von einer
for-Schleife umschlossen ist, schaut wie oft ein x-Wert in der Formel vorkommt.

````c++
vector<vector<int>> lgsNachChi;
for (int i = 0; i < xyFormeln.size(); i++) {
    vector<int> tmp;

    for (int n = 0; n < xyFormeln.at(i).size(); n++) {
        for (int o = 0; o < xyFormeln.at(i).at(n).size() - 1; o++) {
            int t = 0;
            t = xyFormeln.at(i).at(n).at(0) * chi.at(xyFormeln.at(i).at(n).at(1));
            if (t > 0) {
                tmp.push_back(xyFormeln.at(i).at(n).at(0));
            }
        }
    }
    lgsNachChi.push_back(tmp);
}

vector<vector<int>> result;
for (int i = 0; i < lgsNachChi.size(); i++) {
    vector<int> tmp;
    sort(lgsNachChi.at(i).begin(), lgsNachChi.at(i).end());

    for (auto it = std::cbegin(lgsNachChi.at(i)); it != std::cend(lgsNachChi.at(i));) {

        int dups = std::count(it, std::cend(lgsNachChi.at(i)), *it);
        if ((dups % 2) == 1)
            tmp.push_back(*it - 1);
        for (auto last = *it; *++it == last;);
    }
    result.push_back(tmp);
}
````

#### Aus xy-Formel die Basis erzeugen

Für jede der erzeugten "xy-Formeln" stellt in der Basis eine Zeile der Matrix dar. Die Zeile enthält dort jeweils eine
eins wo auch ein x-Wert in der Formel steht.

````text
Gekürzte Formel:
 x_1 + x_2 + x_3
 x_1 + x_3

Sich ergebende Matrix
 [[1 1 1]
  [1 0 1]]
````

Nach dem die Matrix aus den gekürzten Formeln erstellt ist wird der Gauß Algohrithmus angewendet.

````c++
NTL::mat_GF2 ende;
ende.SetDims(lgsNachChi.size(), breite);
for (int i = 0; i < result.size(); i++) {
    for (int n = 0; n < result.at(i).size(); n++) {
        ende[i][result.at(i).at(n)] = 1;
    }
}

NTL::gauss(ende);
````

### Chitext entschlüsseln und überprüfen

Schlussendlich wird die Methode des aufrollen auf die kleine Matrix angewendet und das ergebniss der freien Variabel ist
der vermutete Klartext. Dies lässt sich dadurch überprüfen in dem man den vermeintlichen Klartext erneut verschlüsselt
und dies mit dem abgefangenen Chitext vergleicht. Wenn die beiden Chitexte übereinstimmen ist der Lösung richtig.

## Bibliotheken

___
Es wurde die mathematische Bibliothek NTL (*NTL: A Library for doing Number Theory*) verwendet.<br>
Weitere Informationen hierzu sind unter diesem *[Link](https://libntl.org/)* zu finden.


