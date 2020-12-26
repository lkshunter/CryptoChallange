#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <NTL/mat_GF2.h>
#include <algorithm>
#include <fstream>
#include <regex>

using namespace std;

/* -----------------------------------
 * Helfer-Methoden für den Angriff
 * -----------------------------------
 * */

/*
 * Entfernt alle definierten character aus einem String
 * */
void removeChar(string &str, char elements[]) {
    for (unsigned int i = 0; i < strlen(elements); ++i) {
        str.erase(std::remove(str.begin(), str.end(), elements[i]), str.end());
    }
}

/*
 * Erzeugt aus einem String eine Vector mit dem Klartext
 * */
vector<int> createVector(string clear, char c) {
    vector<int> out;
    stringstream test(clear);
    string segment;

    while (getline(test, segment, c)) {
        out.push_back(stoi(segment));
    }

    return out;
}

/*
 * Erzeugt aus einem char Pointer
 * */
vector<vector<vector<int>>> createPublicKey(string &pKey) {

    /*
     * Logischer Aufbau des publicKey
     *
     * vector1_PublicKey(
     *      vector2_Funktion_1(
     *          vector3_multplikatio_1(
     *              1,2
     *          ),
	 *		    vector3_multplikatio_2(
	 * 			    3,5
	 *		    ),
	 *		    vector3_multplikatio_3(
	 *			    3
	 *		    ),
	 * 		    vector3_multplikatio_4(
	 *			    4
	 *		    ),
	 *		    ...
	 *		    vector3_multplikatio_n(
	 *			    ...
	 *		    )
	 *      ),
	 *      ...
	 *      vector2_Funktion_n(
	 *	        ...
	 *      )
     *  )
     * */

    /*
     * Entfernt chars die nicht für die weiter verarbeitung benötigt werden
     * */
    char listOfUnwantedChars[] = "\n _x";
    removeChar(pKey, listOfUnwantedChars);


    /*
     * Teilt den String an einem ',' in die verschiedenen Funktionen auf und speichert sie in einen Vector
     * */
    stringstream test(pKey);
    string segment;
    vector<string> seglist;

    while (getline(test, segment, ',')) {
        seglist.push_back(segment);
    }

    /*
     * Teilt jede Funktion in ihre Summanden auf
     * */
    vector<vector<string>> list;
    for (vector<string>::iterator it = seglist.begin(); it != seglist.end(); ++it) {

        stringstream test1(*it);
        string segment;
        vector<string> temp;

        // Getrennt wird ab jedem '+'
        while (getline(test1, segment, '+')) {
            temp.push_back(segment);
        }
        list.push_back(temp);

    }

    /*
     * Die Summanden der Formel können ein einzelnes x sein oder eine Multiplikation aus zwei x
     *  - In dem Fall einer Multiplikation wird diese in die einzelnen Multipikaten aufgeteilt
     *
     * Ob nun eine Multiplikation oder nur ein Summand vorliegt ist egal sie werden in einem Vector angelegt
     *  - Die Ziffern werden wenn vorhanden an einem '*' aufgeteilt
     *  - Die Ziffern werden in einen Integer gewandelt
     * */
    vector<vector<vector<int>>> key;
    for (int i = 0; i < list.size(); i++) {

        vector<vector<int>> temp2;
        string segment;
        for (vector<string>::iterator it = list.at(i).begin(); it != list.at(i).end(); ++it) {
            vector<int> temp1;
            stringstream test2(*it);

            while (getline(test2, segment, '*')) {
                temp1.push_back(stoi(segment));
            }
            temp2.push_back(temp1);
        }
        key.push_back(temp2);
    }
    return key;
}

/*
 * Erzeugt für eine bestimmt Matrixbreite ausreichend zufälligen Klartext
 * */
vector<int> generateRandomClear(int Range, int mode) {
    vector<int> ran;
    int max = 0;

    switch (mode) {
        case 1:
            cout << Range << " = " << Range * (Range * Range) << endl;
            max = Range * (Range * Range);

            for (int i = 0; i < max; i++) {
                ran.push_back(rand() % 2);
            }
            break;
        case 2:
            for (int i = 0; i < Range; i++) {
                ran.push_back(rand() % 2);
            }
            break;
        default:
            cout << Range << " = " << (2 * pow(Range, Range)) * Range << endl;
            max = (2 * pow(Range, 2)) * Range;

            for (int i = 0; i < max; i++) {
                ran.push_back(rand() % 2);
            }
            break;
    }

    return ran;
}

/*
 * Ließt eine Datei ein und gibt den Public Ke und den Chitext als string zurück
 * */
vector<string> readChallengeIn(string filename) {
    // Linux
    ifstream t("/mnt/c/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);

    // Windows
    //ifstream t("C:/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);


    std::string str;

    if (!t.eof()) {
        ostringstream ss;
        ss << t.rdbuf(); // reading data
        str = ss.str();
    }

    char chars[] = "\n x_\t";

    for (unsigned int i = 0; i < strlen(chars); ++i) {
        str.erase(std::remove(str.begin(), str.end(), chars[i]), str.end());
    }

    stringstream test(str);
    string segment;
    vector<string> seglist;

    while (getline(test, segment, '[')) {
        seglist.push_back(segment);
    }

    vector<string> tmp;
    for (int i = 1; i < seglist.size(); i++) {
        stringstream test1(seglist.at(i));
        string segment1;
        while (getline(test1, segment1, ']')) {
            tmp.push_back(segment1);
        }

    }

    vector<string> result;
    result.push_back(tmp.at(0));
    result.push_back(tmp.at(2));

    return result;
}

/*
 * -----------------------------------------------
 * Methoden die essenziell für den Angriff sind
 * -----------------------------------------------
 * */

/*
 * Verschlüsselt einen Klartext mit einem Public Key
 * */
vector<int> encrypt(vector<int> &clear, vector<vector<vector<int>>> &pKey) {
    vector<int> chi;
    int cipher = 0;

    /*
     * Geht den gesamten Klartext durch und springt durch die Breite des Public Key weiter
     * */
    for (int o = 0; o < clear.size(); o += pKey.size()) {

        /*
         * Geht jede Funktion nacheinander durch
         * */
        for (int i = 0; i < pKey.size(); i++) {
            cipher = 0;

            /*
             * Geht alle Multiplicationen durch und addiert die einelnen Ergebnisse
             *
             * Beispiel x_2 * x_4
             * */
            for (int j = 0; j < pKey.at(i).size(); j++) {
                vector<int> teilFunktion = pKey.at(i).at(j);

                /*
                 * Da nicht jeder Teil der Funktion aus zwei Operanden besteht muss daruf geprüft werden
                 *
                 * Beispiel x_2 * x_4 oder x_3
                 * */
                if (teilFunktion.size() == 2) {
                    cipher += clear.at(o + teilFunktion.at(0) - 1) * clear.at(o + teilFunktion.at(1) - 1);
                } else if (teilFunktion.size() == 1) {
                    cipher += clear.at(o + teilFunktion.at(0) - 1);
                }
            }

            /*
             * Das Ergebniss der gesamten Funktion wird Modulo 2 gerechnet und in den Vector als chiText gespeichert
             * */
            cipher = cipher % 2;
            chi.push_back(cipher);
        }
        cout << "Zeile: " << o + 1 << " wurde verschlüsselt" << endl;
    }
    return chi;
}

/*
 * Erzeugt aus dem Klar-Geheim Kompromiss die Stufenmatrix
 * */
NTL::mat_GF2 createTriangleMatrix(vector<int> &clear, vector<int> &chi, int l) {

    NTL::mat_GF2 triangle;
    triangle.SetDims((clear.size() / l), l * l);
    NTL::clear(triangle);
    int m, n = 0;

    /*
     * Definieren der benötigten Matrizen/ den Vector
     *
     * Auch wenn der Klartext als Vektor zu versteh ist wird er als Datentyp Matrix_GF2 definiert
     * Programmiertechnich macht es keine Unterschied ob es sich um eine Matrix oder einen Vektor handelt
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

        // Setzt alle Felder auf 0
        NTL::clear(vec);
        NTL::clear(mat);
        NTL::clear(erg);

        // Füllen der Matrix und des Vectors
        for (int j = i; j < (i + l); j++) {
            vec[j - i][0] = clear.at(j);
            mat[0][j - i] = chi.at(j);
        }

        /*
         * Multiplikation von Matrix und Vector zum erzeugen des Klar-Geheim Kompromisses
         *
         * Ergebniss in
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

    /*
     * Der Gauß Algorithmus ist von der Bibliothek NTL implementiert
     *
     * Im Gegensatz zu dem Beispielangriffen ist hier nicht die "reduced row echolon" version verwendet
     * Der Unterschied ist das über der Stufenform mehr einsen stehen können
     * */
    NTL::gauss(triangle);

    cout << triangle << "\n" << endl;

    return triangle;
}

/*
 * Errechnet aus einer Gauß Matrix die speziellen Lösungen aller freien Variablen
 *
 * todo: Muss verbessert werden
 * todo: Kommentieren
 * */
vector<NTL::mat_GF2> matrixAufrollen(NTL::mat_GF2 &gaussMatrix) {
    vector<NTL::mat_GF2> spezLoes;

    /*
     * Die Schleife findet die letzte Zeile die nicht nur aus Nullen besteht
     *
     * todo: Extrem doof geschrieben und Laufzeit intensiv. Muss ersetzt werden durch eine Errechnung des Grades der Matrix. Und das mit den breaks ist auch übel Kacke
     *  - Wenn Zeit ist
     * */
    int rowMax = -1;
    int test = 0;
    for (int i = 0; i < gaussMatrix.NumRows(); i++) {

        for (int j = 0; j < gaussMatrix.NumCols(); j++) {
            if (NTL::IsZero(gaussMatrix[i][j])) {
                test++;
            }
            if (test == gaussMatrix.NumCols()) {
                rowMax = i - 1;
                break;
            }
        }

        if (rowMax != -1) {
            break;
        }
        test = 0;
    }
    // Als oben was schiefgelaufen ist oder die Matrix keine freie Variabel hat
    if (rowMax == -1) {
        rowMax = gaussMatrix.NumRows() - 1;
    }

    /*
     * Findet und speichert alle freien Variabeln
     * */
    if (!(NTL::IsZero(gaussMatrix))) {
        // Speichert die Zeile und Spalter an der die nächste Eins der Stufen Form vermutet wird
        vector<int> expectedOne;

        // Speichert Welche freien Variabeln gefunden wurden
        vector<int> freeVar;
        // Speichert in welcher Zeile eine oder Mehrere freie Variabeln gefunden wurden
        vector<int> rowJump;

        // Die erste 1 wird immer in der lezten Zeile (in der EInsen vorkommen) ganz rechts vermutet
        expectedOne.push_back(rowMax - 1);
        expectedOne.push_back(gaussMatrix.NumCols() - 1);

        /*
         * Die Matrix wird von unten rechts bis oben links durchgegangen
         *
         * todo: Aufwendige Schleifendurchläufe vermeiden
         * */
        for (int i = rowMax; i >= 0; i--) {

            /*
             * Wenn an der vermuteten Stelle eine Eins gefunden wurde
             * */
            if (!(NTL::IsZero(gaussMatrix[i][expectedOne.at(1)]))) {
                int check = 0;
                int indexOfNull = -1;

                /*
                 *
                 * */
                for (int o = expectedOne.at(1) - 1; o >= 0; o--) {
                    if (!(NTL::IsZero(gaussMatrix[i][o]))) {
                        check++;
                        indexOfNull = o;
                    }
                }

                if (check != 0) {

                    for (int x = expectedOne.at(1) - 1; x > (indexOfNull - 1); x--) {
                        freeVar.push_back(expectedOne.at(1));
                        rowJump.push_back(i);
                        expectedOne.at(1)--;
                    }

                    expectedOne.at(1) = indexOfNull - 1;
                } else {
                    expectedOne.at(1)--;
                }

            } else {
                int indexOfNull = -1;

                for (int o = expectedOne.at(1); o >= 0; o--) {
                    if (!(NTL::IsZero(gaussMatrix[i][o]))) {
                        indexOfNull = o;
                    }
                }

                for (int x = expectedOne.at(1) - 1; x > (indexOfNull - 1); x--) {
                    freeVar.push_back(expectedOne.at(1));
                    rowJump.push_back(i);
                    expectedOne.at(1)--;
                }

                expectedOne.at(1) = indexOfNull - 1;

            }

        }

        /*
         * Schreibt alle freien Variabeln auf die Konsole aus
         * */
        cout << "Liste aller freien Variablen: " << endl;
        for (int v = 0; v < freeVar.size(); v++) {
            cout << "x_" << freeVar[v] + 1 << endl;
        }
        cout << endl;

        /*
         * Erzeugt alle speziellen Lösungen für die freien Variabeln
         * */
        cout << "Liste aller Speziellen Lösungen: " << endl;
        for (int v = 0; v < freeVar.size(); v++) {
            // Lösungsvektor
            NTL::mat_GF2 vec;
            vec.SetDims(gaussMatrix.NumCols(), 1);

            // Setzt die freie Variabel auf 1
            vec[freeVar[v]][0] = 1;

            /*
             * Geht die Matrix ab der letzt Zeile mit Einsen von unten nach oben durch
             * */
            int index = gaussMatrix.NumCols() - 1;
            for (int w = rowMax; w >= 0; w--) {
                int test = 0;

                /*
                 * Wenn eine oder mehrere freie Variabelen in einer Zeile gefunden wurden muss der muss der Index im Lösungsvekor weiter springen
                 * */
                for (int g = 0; g < rowJump.size(); g++) {
                    if (w == rowJump[g]) {
                        index--;
                    }
                }

                /*
                 * Rechnet die Zeile zusammen
                 * */
                for (int g = gaussMatrix.NumCols() - 1; g >= 0; g--) {
                    if (!(NTL::IsZero(gaussMatrix[w][g]))) {
                        if (!(NTL::IsZero(vec[g][0]))) {
                            test += 1;
                        }
                    }
                }

                // Je nach ergebnis der Zeile muss der Wert im Lösungsvektor auf 1 gesetzt werden
                if ((test % 2) == 1) {
                    vec[index][0] = 1;
                }
                index--;
            }

            // Speichert die Lösung und gibt sie aus
            spezLoes.push_back(vec);
            cout << vec << "\n" << endl;
        }
    }

    return spezLoes;
}

/*
 * Erzeugt aus eiem Vekor mit speziellen Lösungen eine Basis
 *
 * todo: Kommentieren
 * */
NTL::mat_GF2 createBasis(vector<NTL::mat_GF2> specialSolution, int breite, vector<int> chi) {
    vector<NTL::mat_GF2> matrizen;

    int offset = 0;
    for (int i = 0; i < breite; i++) {

        NTL::mat_GF2 mat;
        mat.SetDims(specialSolution.size(), breite);

        for (int n = 0; n < specialSolution.size(); n++) {
            for (int o = 0; o < breite; o++) {
                mat[n][o] = (specialSolution.at(n))[o + offset][0];
            }
        }

        matrizen.push_back(mat);
        offset += breite;
        //cout << mat << endl;
    }

    vector<vector<vector<int>>> xyFormel;

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
        xyFormel.push_back(tmp2);
    }

    vector<vector<int>> lgsNachChi;

    for (int i = 0; i < xyFormel.size(); i++) {
        vector<int> tmp;

        for (int n = 0; n < xyFormel.at(i).size(); n++) {
            for (int o = 0; o < xyFormel.at(i).at(n).size() - 1; o++) {
                int t = 0;
                t = xyFormel.at(i).at(n).at(0) * chi.at(xyFormel.at(i).at(n).at(1));
                if (t > 0) {
                    tmp.push_back(xyFormel.at(i).at(n).at(0));
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

    NTL::mat_GF2 ende;
    ende.SetDims(lgsNachChi.size(), breite);

    for (int i = 0; i < result.size(); i++) {
        for (int n = 0; n < result.at(i).size(); n++) {
            ende[i][result.at(i).at(n)] = 1;
        }
    }

    NTL::gauss(ende);

    cout << ende << "\n" << endl;

    return ende;

}

/*
 * Steuert den Angriff und vereint die verschiedenen Methoden
 *
 * todo: Kommentieren
 * */
bool angriff(vector<vector<vector<int>>> &publicK, vector<int> &clear, vector<int> &chi) {

    /*
     * Erzeugt aus dem String den für die Berechnung benötigten Vector
     * */

    // Erzeugen des Chitextes
    vector<int> chiText;
    chiText = encrypt(clear, publicK);

    NTL::mat_GF2 a;
    a = createTriangleMatrix(clear, chiText, publicK.size());

    vector<NTL::mat_GF2> specialSolution;
    specialSolution = matrixAufrollen(a);

    if (specialSolution.size() == 0) {
        cout << "Der Key hat einen defekt" << endl;
        return false;
    }

    NTL::mat_GF2 b;
    b = createBasis(specialSolution, publicK.size(), chi);

    vector<NTL::mat_GF2> c;
    c = matrixAufrollen(b);

    if (c.size() == 0 || c.size() > 1) {
        cout << "Es gibt keine Freie variabel in der speziellen Lösung der Basis" << endl;
        return false;
    }

    vector<int> loesung;
    for (int i = 0; i < c.at(0).NumRows(); i++) {
        if (!(NTL::IsZero(c.at(0).get(i, 0)))) {
            loesung.push_back(1);
        } else {
            loesung.push_back(0);
        }
    }

    vector<int> probe = encrypt(loesung, publicK);

    cout << "\nProbe: \t";
    for (int i = 0; i < probe.size(); i++) {
        cout << probe.at(i) << " ";
    }
    cout << endl;
    cout << "Chi: \t";
    for (int i = 0; i < chi.size(); i++) {
        cout << chi.at(i) << " ";
    }
    cout << endl;

    bool success = false;

    if (probe == chi) {
        cout << "Probe bestanden" << endl;
        success = true;
    } else {
        cout << "Probe nicht bestanden" << endl;
    }

    cout << "Lösung: ";
    for (int i = 0; i < c.at(0).NumRows(); i++) {
        cout << c.at(0)[i][0] << " ";
    }
    cout << endl;

    return success;
}

/*
 * -----------------------------------------
 * Verschiedene Methoden die einen Angriff auslösen: dynamische, vorgegebene und hardgecodede
 * -----------------------------------------
 * */

/*
 * Hardgecodeter Beispielangriff d3
 * */
void angriffD3() {
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

/*
 * Führt einen Angriff mit den Crypto Challenge Vorgaben aus dem Moodle Kurs durch.
 *
 * todo: Kommentieren
 * */
void angriffGruppeN(int gruppe, int mode) {

    if (gruppe > 0 && gruppe <= 14) {

        string file = "kryptochallengegruppe";
        file += to_string(gruppe);
        file += ".txt";

        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        vector<string> input = readChallengeIn(file);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        long misec = chrono::duration_cast<chrono::microseconds>(end - start).count();

        std::cout << misec / 1000 << ":" << misec % 1000 << " [ms:μm]\n" << endl;

        vector<vector<vector<int>>> publicK = createPublicKey(input.at(0));

        if (mode == 1) {
            cout << "Es wurde der Performance Modus eingestellt." << endl;
            cout << "Der Klar-Geheim Kompromiss liegt unter 2*n^2 es kann sein das kein Ergebniss gefunden wird."
                 << endl;
            cout << "" << endl;
        } else {
            cout << "Der Klar-Geheim Kompromiss wird mit 2*n^2 durchgeführt." << endl;
        }

        vector<int> clear = generateRandomClear(publicK.size(), mode);
        vector<int> chi = createVector(input.at(1), ',');

        angriff(publicK, clear, chi);

    } else {
        cout << "Ein Angriff für die Gruppe " << gruppe
             << " existiert nicht.\n Bitte wähle eine Zahl im bereich von einschließlich 1 bis 14" << endl;
    }
}

/*
 * Führt einen Angriff auf einen beliebigen Public Key und Chitext, aus einer entsprechend formatierten Datei, aus
 *
 * todo: Kommentieren
 * */
bool angriffDatei(string file, int mode) {

    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    vector<string> input = readChallengeIn(file);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    long misec = chrono::duration_cast<chrono::microseconds>(end - start).count();

    std::cout << misec / 1000 << ":" << misec % 1000 << " [ms:μm]\n" << endl;

    if (mode == 1) {
        cout
                << "Es wurde der Performance Modus eingestellt.\nDer Klar-Geheim Kompromiss liegt unter 2*n^2.\nEs kann sein das kein Ergebniss gefunden wird."
                << endl;
        cout << "" << endl;
    } else {
        cout << "Der Klar-Geheim Kompromiss wird mit 2*n^2 durchgeführt." << endl;
    }

    vector<vector<vector<int>>> publicK = createPublicKey(input.at(0));
    vector<int> clear = generateRandomClear(publicK.size(), mode);
    vector<int> chi = createVector(input.at(1), ',');

    return angriff(publicK, clear, chi);
}

/*
 * -------------------------------------------------
 * Methoden die Ideen, Versuche, Übungen umsetzen
 * -------------------------------------------------
 * */

/*
 * Erzeugt einen Public Key und dazu gehörigen Chitext
 *
 * Funktioniert leider nicht. Wahrscheinlich fehlt der mathematische Hintergrund.
 * */
void generateRandomChallange(int dimension) {
    string ranKey = "";

    for (int i = 0; i < dimension; i++) {

        int end = (rand() % (dimension * dimension)) + dimension;
        for (int n = 0; n < end; n++) {
            if ((rand() % 5) == 1) {
                ranKey += "x_";
                ranKey += to_string((rand() % dimension) + 1);
            } else {
                ranKey += "x_";
                ranKey += to_string((rand() % dimension) + 1);
                ranKey += "*x_";
                ranKey += to_string((rand() % dimension) + 1);
            }

            if (n < (end - 1)) {
                ranKey += " + ";
            } else {
                ranKey += ",\n";
            }
        }
    }

    // Erzeugt einen zufälligen Namen
    string filename = "selfGenerated_d" + to_string(dimension) + "_" + to_string(rand() % 100000) + ".txt";

    // Linux Pfad
    std::ofstream outfile("/mnt/c/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);
    // Windows Pfad
    //std::ofstream outfile("C:/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);

    // Erzeugt zu dem generierten Key einen Chitext
    vector<vector<vector<int>>> fancyKey = createPublicKey(ranKey);
    vector<int> clear = generateRandomClear(dimension, 2);
    vector<int> chi = encrypt(clear, fancyKey);

    // Schreibt den Key und den Chitext in eine Datei
    outfile << "Public Key: [ " << ranKey << "]" << endl;
    outfile << "Chitext: [ ";
    for (int i = 0; i < chi.size(); i++) {
        if (i < (chi.size() - 1)) {
            outfile << chi.at(i) << ", ";
        } else {
            outfile << chi.at(i);
        }
    }
    outfile << "]" << endl;
}


int main() {
    srand(time(0));

    /*
     * Bei Problemen unter Linux bitte die readme.md lesen
     * */

    // Start der Zeitmessung
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    /*
     * Der Mode gibt an wie viel Klar-Geheim Kompromiss verwendet werden soll
     *
     * 0 = Normaler Modus: Wie in der Vorlesung besprochen (2*n^2) * n
     * 1 = Performance Modus: Weniger als  beim "Normalen Modus" (n*n^2) Kann zu fehler führen
     *
     * Nicht zu verwenden:
     * [2 = Eine Zeile (n) wird intern verwendet]
     * */
    int mode = 1;

    /*
     * Jeder in Moodel zu verfügung gestellter Angriff kann hier durchgeführt werden
     * */
    angriffGruppeN(1, mode);

    /*
     * Theoretisch kan jeder richtig formatierte Datei eingelesen werden
     * */
    //angriffDatei("BeispielAngriffd7.txt", mode);

    /*
     * Exemplarisch ist hier einer der Beispielangriffe hardgecoded
     *
     * Kein mode da der Klartext vorgegeben war im Beispiel
     * */
    //angriffD3();


    // Ende der Ausführung wird gespeichert
    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    /*
     * Ausgabe der Zeit die für die Berechnung benötigt wurde
     * */
    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();
    cout << "Benötigte Zeit:" << endl;
    cout << (((misec / 1000) / 1000) / 60) % 60 << ":" << ((misec / 1000) / 1000) % 60 << ":" << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;

    return 0;
}
