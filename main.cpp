#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <NTL/mat_GF2.h>
#include <algorithm>
#include <fstream>
#include <thread>
#include <pthread.h>

using namespace std;

/**
 * Globale Variable
 * */
string glb_path;

/* -----------------------------------
 * Helfer-Methoden für den Angriff
 * -----------------------------------
 * */

/**
 * Entfernt alle definierten Character aus einem String
 *
 * @category helfer
 * @param str: String aus dem die Zeichen entfernt werden sollen
 * @param elements: Char[] in dem alle zu löschenden character enthalten sind
 * */
void removeChar(string &str, char elements[]) {
    for (unsigned int i = 0; i < strlen(elements); ++i) {
        str.erase(std::remove(str.begin(), str.end(), elements[i]), str.end());
    }
}

/**
 * Trennt den String mit dem Klartext an einem Character auf. Die Ziffern werden in einen int gecastet
 *
 * @category helfer
 * @param clear: String der zerteilt werden soll
 * @param c: Der Character an dem der String aufgetrennt werden soll
 * @return Einen Vector mit dem Klartext als einzelne int-Werte
 * */
vector<int> createVector(string &clear, char c) {
    vector<int> out;
    stringstream test(clear);
    string segment;

    while (getline(test, segment, c)) {
        out.push_back(stoi(segment));
    }

    return out;
}

/**
 * Erzeugt aus einem Public Key der als String eingelesen wurde einen iterierbaren Vector
 *
 * @category helfer
 * @param pKey: Der Public Key als String
 * @return Den Public Key als Vector
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

/**
 * Erzeugt für eine bestimmt Matrixbreite ausreichend zufälligen Klartext
 *
 * @category helfer
 * @param d: Wie viele Bits können in einem mal verschlüsselt werden
 * @param mode: Gibt an in welchem Modus Klartext erzeugt werden soll
 * @return Klartext der Verschlüsselt werden kann
 * */
vector<int> generateRandomClear(int d, int mode) {
    vector<int> ran;
    long max = 0;

    switch (mode) {
        case 0:
            max = (2 * pow(d, 2)) * d;

            for (int i = 0; i < max; i++) {
                ran.push_back(rand() % 2);
            }
            break;
        case 1:
            max = d * (d * d);

            for (int i = 0; i < max; i++) {
                ran.push_back(rand() % 2);
            }
            break;
        case 2:
            for (int i = 0; i < d; i++) {
                ran.push_back(rand() % 2);
            }
            break;
        default:
            cout << "Es wurde ein ungültiger Modus gewählt. Es wird standardmäßig 2*n^2 genutzt" << endl;
            max = (2 * pow(d, 2)) * d;

            for (int i = 0; i < max; i++) {
                ran.push_back(rand() % 2);
            }
            break;
    }

    return ran;
}

/**
 * Ließt eine Datei ein und gibt den Public Ke und den Chitext als string zurück
 *
 * @category helfer
 * @param filename: Der Dateiname samt Endung der Datei die eingelesen werden soll
 * @return An stelle 0 im Vector steht der Key als das String und an Stelle 1 der Chitext
 * */
vector<string> readChallengeIn(string &filename) {
    // Datei einlesen
    ifstream t(glb_path + filename);

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

/**
 * Lamda Funktion für die parallelisierung der Verschlüsselung
 *
 * @param id: Nummer des Threads
 * @param clear: Die Speicheradresse des gesamten Klartextvektors
 * @param start: Ab diesem Index im Klartextvektor ist der Thread für die Verschlüsselung zuständig
 * @param end: Bis zu diesem Index im Klartextvektor ist der Thread für die Verschlüsselung zuständig
 * @param result: In die Speicheradresse des übergeben Vektor werden die verschlüsselten Bits gespeichert
 * @param pKey: Die Speicheradresse des Public Key's
 * */
auto encyptDistributed = [](int id, vector<int> &clear, int start, int end, vector<int> &result,
                            vector<vector<vector<int>>> &pKey) {
    int cipher = 0;

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    /*
     * Geht den gesamten Klartext durch und springt durch die Breite des Public Key weiter
     * */
    for (int o = start * pKey.size(); o < (end * pKey.size()); o += pKey.size()) {

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

                /*
                 * Da nicht jeder Teil der Funktion aus zwei Operanden besteht muss daruf geprüft werden
                 *
                 * Beispiel x_2 * x_4 oder x_3
                 * */
                if (pKey.at(i).at(j).size() == 2) {
                    cipher += clear.at(o + pKey.at(i).at(j).at(0) - 1) * clear.at(o + pKey.at(i).at(j).at(1) - 1);
                } else {
                    cipher += clear.at(o + pKey.at(i).at(j).at(0) - 1);
                }
            }

            /*
             * Das Ergebniss der gesamten Funktion wird Modulo 2 gerechnet und in den Vector als chiText gespeichert
             * */
            cipher = cipher % 2;
            result.push_back(cipher);
        }
    }

    chrono::steady_clock::time_point ende = chrono::steady_clock::now();

    long misec = chrono::duration_cast<chrono::microseconds>(ende - begin).count();

    cout << "\tThread " << id << " hat " << (end * pKey.size()) - (start * pKey.size()) << " Bits verschlüsselt: \n\t\t"
         << (((misec / 1000) / 1000) / 60) % 60 << ":" << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;
};

/**
 * Verschlüsselt einen Klartext mit einem Public Key
 *
 * @category angriff
 * @param clear: Klartext als vector<int>
 * @param pKey: Public Key als vector<vector<vector<int>>>
 * @return Chitext als vector<int>
 * */
vector<int> encryptSingleThread(vector<int> &clear, vector<vector<vector<int>>> &pKey) {
    vector<int> chi;
    int cipher = 0;

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

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
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

    cout << "Singlethread verschlüsseln abgeschlossen nach: \n\t" << (((misec / 1000) / 1000) / 60) % 60 << ":"
         << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;

    return chi;
}

/**
 * Verschlüsselt einen Klartext mit einem Public Key
 *
 * @category angriff
 * @param clear: Klartext als vector<int>
 * @param pKey: Public Key als vector<vector<vector<int>>>
 * @return Chitext als vector<int>
 * */
vector<int> encryptMultiThreadX4(vector<int> &clear, vector<vector<vector<int>>> &pKey) {

    cout << clear.size() << " Bits - Verschlüsseln des Klartextes in Threads: " << endl;

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

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    thread task1(encyptDistributed, 1, std::ref(clear), 0, range - diff3, std::ref(results.at(0)), std::ref(pKey));
    thread task2(encyptDistributed, 2, std::ref(clear), range, (range * 2) - diff2, std::ref(results.at(1)),
                 std::ref(pKey));
    thread task3(encyptDistributed, 3, std::ref(clear), range * 2, (range * 3) - diff1, std::ref(results.at(2)),
                 std::ref(pKey));
    thread task4(encyptDistributed, 4, std::ref(clear), range * 3, ((range * 4) - diff), std::ref(results.at(3)),
                 std::ref(pKey));

    task4.join();
    task1.join();
    task2.join();
    task3.join();

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

    cout << (results.at(0).size() + results.at(1).size() + results.at(2).size() + results.at(3).size())
         << " Bits - Multithreading verschlüsseln abgeschlossen nach: \n\t" << (((misec / 1000) / 1000) / 60) % 60
         << ":"
         << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;

    vector<int> res;
    for (int i = 0; i < results.size(); i++) {
        for (int n = 0; n < results.at(i).size(); n++) {
            res.push_back(results.at(i).at(n));
        }
    }

    return res;
}

/**
 * Erzeugt aus dem Klar-Geheim Kompromiss die Stufenmatrix
 * */
NTL::mat_GF2 createTriangleMatrix(vector<int> &clear, vector<int> &chi, int l) {

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    NTL::mat_GF2 triangle;
    triangle.SetDims((clear.size() / l), l * l);
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

    /*
     * Der Gauß Algorithmus ist von der Bibliothek NTL implementiert
     *
     * Im Gegensatz zu dem Beispielangriffen ist hier nicht die "reduced row echolon" version verwendet
     * Der Unterschied ist das über der Stufenform mehr einsen stehen können
     * */
    NTL::gauss(triangle);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

    cout << "KGK erzeugen und Gauss anwenden abgeschlossen nach: \n\t" << (((misec / 1000) / 1000) / 60) % 60 << ":"
         << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;

    return triangle;
}

/**
 * Lamda
 * */
auto solveFreeVarDistributed = [](int id, vector<int> &freeVar, int start, int end, int rowMax, vector<int> &rowJump,
                                  NTL::mat_GF2 &gaussMatrix, vector<NTL::mat_GF2> &spezLoes) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    for (int i = start; i < end; i++) {
        // Lösungsvektor
        NTL::mat_GF2 vec;
        vec.SetDims(gaussMatrix.NumCols(), 1);

        // Setzt die freie Variabel auf 1
        vec[freeVar[i]][0] = 1;

        /*
         * Geht die Matrix ab der letzt Zeile mit Einsen von unten nach oben durch
         * */
        int index = gaussMatrix.NumCols() - 1;
        for (int n = rowMax; n >= 0; n--) {
            int rowResult = 0;

            /*
             * Wenn eine oder mehrere freie Variabelen in einer Zeile gefunden wurden muss der muss der Index im Lösungsvekor weiter springen
             * */
            for (int o = (rowJump.size()) - 1; o >= 0; o--) {
                if (n == rowJump[o]) {
                    index--;
                }
            }

            /*
             * Checkt alle Einsen in der Zeile und sucht den entsprechenden Wert aus dem Lösungsvektor
             * Die Werte aus dem Lösungsvektor werden addiert
             * */
            for (int o = gaussMatrix.NumCols() - 1; o >= 0; o--) {
                if (!(NTL::IsZero(gaussMatrix[n][o]))) {
                    if (!(NTL::IsZero(vec[o][0]))) {
                        rowResult += 1;
                    }
                }
            }

            /*
             * Jede zeile muss NUll ergeben
             *
             * Wenn das Ergebnis "1" ist so muss die Stufenform Eins im Lösungsvektor auf "1" gesetzt werden
             * */
            if ((rowResult % 2) == 1) {
                vec[index][0] = 1;
            }
            index--;
        }
        //cout << vec << endl;
        spezLoes.push_back(vec);
    }

    chrono::steady_clock::time_point ende = chrono::steady_clock::now();

    long misec = chrono::duration_cast<chrono::microseconds>(ende - begin).count();

    cout << "\tThread " << id << " hat " << end - start << " spezielle Lösungen errechnet: \n\t\t"
         << (((misec / 1000) / 1000) / 60) % 60 << ":" << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;
};

/**
 * Errechnet aus einer Gauß Matrix die speziellen Lösungen aller freien Variablen
 *
 * @category angriff
 * @param gaussMatrix: Eine Matrix die schon in der Stufenzeilenform ist wird erwartet
 * @return Verktor mit allen Speziellen Lösungen einer Gauss Matrix
 * */
vector<NTL::mat_GF2> matrixAufrollenSingleThread(NTL::mat_GF2 &gaussMatrix) {

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<NTL::mat_GF2> spezLoes;
    vector<vector<int>> oneSteps;
    int rowMax = -1;

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
    // Als oben was schiefgelaufen ist oder die Matrix keine freie Variabel hat
    if (rowMax <= 0) {
        return spezLoes;
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
        expectedOne.push_back(rowMax);
        expectedOne.push_back(gaussMatrix.NumCols() - 1);

        for (int i = rowMax; i >= 0; i--) {
            /*
             * Ist die vermutete Eins auch die erste in der Zeile
             * */
            if (!(expectedOne.at(0) == oneSteps.at(i).at(0) && expectedOne.at(1) == oneSteps.at(i).at(1))) {
                /*
                 * Speichert alle Spalten zwischen der vermuteten und ersten eins und der Realen als freie
                 * Die vermutete Eins wird auch als freie Variable gespeichert die Stufenform Eins nicht
                 * */
                for (int n = expectedOne.at(1); n > oneSteps.at(i).at(1); n--) {
                    rowJump.push_back(i);
                    freeVar.push_back(n);
                }

                /*
                 * Verhindert out of bound
                 * */
                if (i > 0) {
                    expectedOne.at(0) = oneSteps.at(i).at(0);
                    expectedOne.at(1) = oneSteps.at(i).at(1);
                }
            }

            /*
             * Die Nächste Eins ist von der Stufenform im optimal Fall eine nach links und einen nach oben
             * */
            expectedOne.at(0) -= 1;
            expectedOne.at(1) -= 1;
        }
/*
        cout << "Liste aller freien Variablen: " << endl;
        for (int i = 0; i < freeVar.size(); i++) {
            cout << "x_" << freeVar[i] + 1 << endl;
        }*/

        /*
         * Erzeugt alle speziellen Lösungen für die freien Variabeln
         *
         * todo: Weniger Iteration wenn möglich (Mir ist noch nichts eingefallen)
         * */
        //cout << "\nListe aller Speziellen Lösungen: " << endl;
        for (int i = 0; i < freeVar.size(); i++) {
            // Lösungsvektor
            NTL::mat_GF2 vec;
            vec.SetDims(gaussMatrix.NumCols(), 1);

            // Setzt die freie Variabel auf 1
            vec[freeVar[i]][0] = 1;

            /*
             * Geht die Matrix ab der letzt Zeile mit Einsen von unten nach oben durch
             * */
            int index = gaussMatrix.NumCols() - 1;
            for (int n = rowMax; n >= 0; n--) {
                int rowResult = 0;

                /*
                 * Wenn eine oder mehrere freie Variabelen in einer Zeile gefunden wurden muss der muss der Index im Lösungsvekor weiter springen
                 * */
                for (int o = (rowJump.size()) - 1; o >= 0; o--) {
                    if (n == rowJump[o]) {
                        index--;
                    }
                }

                /*
                 * Checkt alle Einsen in der Zeile und sucht den entsprechenden Wert aus dem Lösungsvektor
                 * Die Werte aus dem Lösungsvektor werden addiert
                 * */
                for (int o = gaussMatrix.NumCols() - 1; o >= 0; o--) {
                    if (!(NTL::IsZero(gaussMatrix[n][o]))) {
                        if (!(NTL::IsZero(vec[o][0]))) {
                            rowResult += 1;
                        }
                    }
                }

                /*
                 * Jede zeile muss NUll ergeben
                 *
                 * Wenn das Ergebnis "1" ist so muss die Stufenform Eins im Lösungsvektor auf "1" gesetzt werden
                 * */
                if ((rowResult % 2) == 1) {
                    vec[index][0] = 1;
                }
                index--;
            }

            /*
             * Speichert die Lösung in vector Und gibt sie auf die Konsole aus
             * */
            spezLoes.push_back(vec);
            //cout << "\nLösung für x_" << freeVar[i] << " = 1\n" << vec << "\n" << endl;
        }

        chrono::steady_clock::time_point end = chrono::steady_clock::now();

        long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

        cout << "Singlethread Matrix aufrollen abgeschlossen nach: \n\t" << (((misec / 1000) / 1000) / 60) % 60 << ":"
             << ((misec / 1000) / 1000) % 60 << ":"
             << (misec / 1000) % 1000
             << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;
    }

    return spezLoes;
}

/**
 * Errechnet aus einer Gauß Matrix die speziellen Lösungen aller freien Variablen
 *
 * @category angriff
 * @param gaussMatrix: Eine Matrix die schon in der Stufenzeilenform ist wird erwartet
 * @return Verktor mit allen Speziellen Lösungen einer Gauss Matrix
 * */
vector<NTL::mat_GF2> matrixAufrollenMultiThreadX4(NTL::mat_GF2 &gaussMatrix) {

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<NTL::mat_GF2> spezLoes;
    vector<NTL::mat_GF2> res;
    vector<vector<int>> oneSteps;
    int rowMax = -1;

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
    // Als oben was schiefgelaufen ist oder die Matrix keine freie Variabel hat
    if (rowMax < 0) {
        return spezLoes;
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
        expectedOne.push_back(rowMax);
        expectedOne.push_back(gaussMatrix.NumCols() - 1);

        for (int i = rowMax; i >= 0; i--) {
            /*
             * Ist die vermutete Eins auch die erste in der Zeile
             * */
            if (!(expectedOne.at(0) == oneSteps.at(i).at(0) && expectedOne.at(1) == oneSteps.at(i).at(1))) {
                /*
                 * Speichert alle Spalten zwischen der vermuteten und ersten eins und der Realen als freie
                 * Die vermutete Eins wird auch als freie Variable gespeichert die Stufenform Eins nicht
                 * */
                for (int n = expectedOne.at(1); n > oneSteps.at(i).at(1); n--) {
                    rowJump.push_back(i);
                    freeVar.push_back(n);
                }

                /*
                 * Verhindert out of bound
                 * */
                if (i > 0) {
                    expectedOne.at(0) = oneSteps.at(i).at(0);
                    expectedOne.at(1) = oneSteps.at(i).at(1);
                }
            }

            /*
             * Die Nächste Eins ist von der Stufenform im optimal Fall eine nach links und einen nach oben
             * */
            expectedOne.at(0) -= 1;
            expectedOne.at(1) -= 1;
        }

        /*
         * Erzeugt alle speziellen Lösungen für die freien Variabeln
         * */

        cout << "Erzeugen der speziellen Lösungen in Threads:" << endl;

        double tmp = (freeVar.size() / 4.0);
        int range = ceil(tmp);
        int diff = ((range * 4) - freeVar.size());
        int diff1 = 0;
        int diff2 = 0;
        int diff3 = 0;


        vector<NTL::mat_GF2> result;
        vector<vector<NTL::mat_GF2>> results;
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

        thread task1(solveFreeVarDistributed, 1, std::ref(freeVar), 0, (range - diff3), rowMax, std::ref(rowJump),
                     std::ref(gaussMatrix), std::ref(results.at(0)));
        thread task2(solveFreeVarDistributed, 2, std::ref(freeVar), range, ((range * 2) - diff2), rowMax,
                     std::ref(rowJump),
                     std::ref(gaussMatrix), std::ref(results.at(1)));
        thread task3(solveFreeVarDistributed, 3, std::ref(freeVar), range * 2, ((range * 3) - diff1), rowMax,
                     std::ref(rowJump),
                     std::ref(gaussMatrix), std::ref(results.at(2)));
        thread task4(solveFreeVarDistributed, 4, std::ref(freeVar), range * 3, ((range * 4) - diff), rowMax,
                     std::ref(rowJump), std::ref(gaussMatrix), std::ref(results.at(3)));

        task4.join();
        task1.join();
        task2.join();
        task3.join();

        chrono::steady_clock::time_point end = chrono::steady_clock::now();

        for (int i = 0; i < results.size(); i++) {
            for (int n = 0; n < results.at(i).size(); n++) {
                res.push_back(results.at(i).at(n));
            }
        }

        long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

        cout << "Multithread Matrix aufrollen abgeschlossen nach: \n\t" << (((misec / 1000) / 1000) / 60) % 60 << ":"
             << ((misec / 1000) / 1000) % 60 << ":"
             << (misec / 1000) % 1000
             << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;
    }

    return res;
}

/**
 * Erzeugt aus einem Vekor mit speziellen Lösungen eine Basis
 *
 * @param specialSolution: Der Vector mit allen speziellen Lösungen
 * @param breite: Wie viele Bits können auf einmal verschlüsselt werden
 * @param chi: Der urspünglich Abgefangene Chitext der Entschlüsselt werden soll
 * @return Gibt die Basis der Speziellen Lösung nach Gauss zurück
 * */
NTL::mat_GF2 createBasis(vector<NTL::mat_GF2> &specialSolution, int breite, vector<int> &chi) {

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<NTL::mat_GF2> matrizen;

    /*
     * Erzeugt aus allen speziellen Lösungen die Matrizen x_1, x_2 , ..., x_n
     * */
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

    /*
     * Aus den Matrizen wird die xy-Formeln erzeugt
     *
     * Für jede Zeile wird eine einzelne Formel erzeugt
     * der x Wert steht für die Matrize in der eine 1 gefunden wurde y steht für die Spalte in der Zeile
     * Jedes Paar xy wird als Multiplikation verstanden und mit einem + an die nächste gekoppelt
     * */
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

    /*
     * Diese Codeabschnitt kürzt die Formel auf ein minimum herunter
     *
     * Es werden die Indexe
     * */
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

    /*
     * Speichert wie haeufig ein x in der Formel vorkommt
     *
     * Wenn die Anzahl modulo 2 gleich 0 ist heben sie sich auf und müssen nicht mehr beachtet werden
     * */
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

    /*
     * Erzeugt aus den Formel eine Matrix und wendet den Gauss Algorithmus drauf an
     * */
    NTL::mat_GF2 ende;
    ende.SetDims(lgsNachChi.size(), breite);
    for (int i = 0; i < result.size(); i++) {
        for (int n = 0; n < result.at(i).size(); n++) {
            ende[i][result.at(i).at(n)] = 1;
        }
    }

    NTL::gauss(ende);

    //cout << ende << endl;

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

    cout << "Lösung der Basis abgeschlossen nach: \n\t" << (((misec / 1000) / 1000) / 60) % 60 << ":"
         << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;

    return ende;

}

/**
 * Steuert den Angriff und vereint die verschiedenen Methoden
 *
 * @param publicK: Public Key
 * @param clear: Klartext als Vector
 * @param chi: Chitext als Vector
 * */
bool angriff(vector<vector<vector<int>>> &publicK, vector<int> &clear, vector<int> &chi, bool multithread) {

    /*
     * Verschlüsselt den Klartext
     * */
    vector<int> chiText;
    if (multithread == true) {
        chiText = encryptMultiThreadX4(clear, publicK);
    } else {
        chiText = encryptSingleThread(clear, publicK);
    }

    /*
     * Errechnet aus dem KGK alle speziellen Lösungen
     * */
    NTL::mat_GF2 kgk = createTriangleMatrix(clear, chiText, publicK.size());

    vector<NTL::mat_GF2> specialSolution = matrixAufrollenMultiThreadX4(kgk);

    /*
     * Wenn es keine spezielle Lösung gibt ist etwas schief gelaufen
     * */
    if (specialSolution.size() == 0) {
        cout << "Der Key hatte möglicherweise einen defekt oder es gab zu wenig Klar-Geheim Kompromiss" << endl;
        return false;
    }

    /*
     * Erzeugt eine Lösung durch aufrollen für die Basis
     * */
    NTL::mat_GF2 rollen = createBasis(specialSolution, publicK.size(), chi);
    vector<NTL::mat_GF2> c = matrixAufrollenSingleThread(rollen);

    /*
     * Wenn es mehr oder weniger als eine Lösung gibt ist etwas schief gelaufen
     * */
    if (c.size() != 1) {
        cout << c.size() << " Der Key hatte möglicherweise einen defekt oder es gab zu wenig Klar-Geheim Kompromiss"
             << endl;
        return false;
    }

    /*
     * Wenn bis hier alles ohne Probleme gelaufen ist wird das Ergebnis überprüft
     * */
    vector<int> loesung;
    for (int i = 0; i < c.at(0).NumRows(); i++) {
        if (!(NTL::IsZero(c.at(0).get(i, 0)))) {
            loesung.push_back(1);
        } else {
            loesung.push_back(0);
        }
    }
    vector<int> probe = encryptSingleThread(loesung, publicK);

    cout << "\nVerschlüsselte Lösung: \t";
    for (int i = 0; i < probe.size(); i++) {
        cout << probe.at(i) << " ";
    }
    cout << "\nOriginal Chitext: \t\t";
    for (int i = 0; i < chi.size(); i++) {
        cout << chi.at(i) << " ";
    }

    bool success = false;

    if (probe == chi) {
        cout << "\n\nProbe bestanden" << endl;
        success = true;
    } else {
        cout << "\n\nProbe nicht bestanden" << endl;
    }

    cout << "\nBestätigte Lösung: ";
    for (int i = 0; i < c.at(0).NumRows(); i++) {
        cout << c.at(0)[i][0] << " ";
    }
    cout << endl;

    return success;
}

/*
 * -------------------------------------------------------------------------------------------
 * Verschiedene Methoden die einen Angriff auslösen: dynamische, vorgegebene und hardgecodede
 * -------------------------------------------------------------------------------------------
 * */

/**
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

    angriff(publicK, klartext, chi, false);
}

/**
 * Führt einen Angriff mit den Crypto Challenge Vorgaben aus dem Moodle Kurs durch.
 *
 * @param gruppe: Gruppennummer
 * @param mode: Modus der bestimmt wie viel Klartext erzeugt wird
 * */
void angriffGruppeN(int gruppe, int mode, bool multiThread) {

    /*
     * Nur Zahlen zwischen 0 und 15 sind zulässig
     * */
    if (gruppe > 0 && gruppe <= 14) {

        // Name der Datei
        string file = "kryptochallengegruppe" + to_string(gruppe) + ".txt";

        // Datei wird eingeleesen
        vector<string> input = readChallengeIn(file);

        // Der Public Key wird aufbereitet
        vector<vector<vector<int>>> publicK = createPublicKey(input.at(0));

        // Je nach Modus wird eine Warnung ausgegeben
        if (mode == 1) {
            cout << "Es wurde der Performance Modus eingestellt.\n"
                 << "Der Klar-Geheim Kompromiss liegt unter 2*n^2. \nEs kann sein das kein Ergebniss gefunden wird.\n"
                 << endl;
        } else if (mode == 2) {
            cout
                    << "Der Klar-Geheim Kompromiss beträgt nur eine Zeile. \nDieser Modus ist nicht für einen Angriff geeignet.\n"
                    << endl;
        } else {
            cout << "Der Klar-Geheim Kompromiss wird mit 2*n^2 durchgeführt.\n" << endl;
        }

        // Es wird eine Klartext generiert
        vector<int> clear = generateRandomClear(publicK.size(), mode);

        vector<int> chi = createVector(input.at(1), ',');

        // Der Angriff wird durchgeführt
        angriff(publicK, clear, chi, multiThread);

    } else {
        // Falls die Zahl nicht einer Gruppe zuzuorden ist
        cout << "Ein Angriff für die Gruppe " << gruppe
             << " existiert nicht.\n Bitte wähle eine Zahl im bereich von einschließlich 1 bis 14" << endl;
    }
}

/**
 * Führt einen Angriff auf einen beliebigen Public Key und Chitext, aus einer entsprechend formatierten Datei, aus
 *
 * @param file: Dateiname samt Endung
 * @param mode: Modus der bestimmt wie viel Klartext erzeugt wird
 * @return Gibt aus ob der Angriff erfolgreich durchgeführt wurde
 * */
bool angriffDatei(string file, int mode, bool multiThread) {
    // Ließt die Datei ein
    vector<string> input = readChallengeIn(file);

    // Je nach Modus wird eine Warnung ausgegeben
    if (mode == 1) {
        cout << "Es wurde der Performance Modus eingestellt.\n"
             << "Der Klar-Geheim Kompromiss liegt unter 2*n^2. \nEs kann sein das kein Ergebniss gefunden wird.\n"
             << endl;
    } else if (mode == 2) {
        cout
                << "Der Klar-Geheim Kompromiss beträgt nur eine Zeile. \nDieser Modus ist nicht für einen Angriff geeignet.\n"
                << endl;
    } else {
        cout << "Der Klar-Geheim Kompromiss wird mit 2*n^2 durchgeführt.\n" << endl;
    }

    /*
     * Bereitet alles für den Angriff vor
     * */
    vector<vector<vector<int>>> publicK = createPublicKey(input.at(0));
    vector<int> clear = generateRandomClear(publicK.size(), mode);
    vector<int> chi = createVector(input.at(1), ',');

    /*
     * Führt den Angriff durch
     * */
    return angriff(publicK, clear, chi, multiThread);
}

/*
 * -------------------------------------------------
 * Methoden die Ideen, Versuche, Übungen umsetzen
 * -------------------------------------------------
 * */

/**
 * Erzeugt einen Public Key und dazu gehörigen Chitext
 *
 * @bug Ja und der ist groß aber ich weiß nicht warum und wie man ihn lösen kann
 *
 * @param dimension: Gibt an wie viele Bits der Key auf einmal verschlüsseln kann
 * @return void: Die Challenge wird in einer Datei abgelegt
 * */
void generateRandomChallenge(int dimension) {
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
    std::ofstream outfile(glb_path + filename);

    // Erzeugt zu dem generierten Key einen Chitext
    vector<vector<vector<int>>> fancyKey = createPublicKey(ranKey);
    vector<int> clear = generateRandomClear(dimension, 2);
    vector<int> chi = encryptSingleThread(clear, fancyKey);

    // Schreibt den Key und den Chitext in eine Datei
    outfile << "Public Key: [ " << ranKey << "]" << endl;
    outfile << "Chitext: [ ";
    for (int i = 0; i < chi.size(); i++) {
        if (i <= (chi.size() - 1)) {
            outfile << chi.at(i) << ", ";
        } else {
            outfile << chi.at(i);
        }
    }
    outfile << "]" << endl;
}


int main() {
    srand(time(0));

    /**
     * Bei Bedarf muss der Pfad an ein anders System angepasst werden.
     * */
    // Windows-Subsystem für Linux
    //glb_path = "/mnt/c/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/";
    // Windows
    glb_path = "C:/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/";

    // Start der Zeitmessung
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    /**
     * Der Mode gibt an wie viel Klar-Geheim Kompromiss verwendet werden soll
     *
     * 0 = Normaler Modus: Wie in der Vorlesung besprochen (2*n^2) * n
     * 1 = Performance Modus: Weniger als  beim "Normalen Modus" (n*n^2) Kann zu fehler führen
     *
     * Nicht zu verwenden:
     * [2 = Eine Zeile (n) wird intern verwendet]
     * */
    int mode = 1;

    /**
     * Jeder in Moodel zu verfügung gestellter Angriff kann hier durchgeführt werden
     * */
    angriffGruppeN(1, mode, true);

    /**
     * Theoretisch kann jeder richtig formatierte Datei eingelesen werden
     * */
    //angriffDatei("BeispielAngriffd7.txt", mode, true);

    /**
     * Exemplarisch ist hier einer der Beispielangriffe hardgecoded
     *
     * Kein mode da der Klartext vorgegeben war im Beispiel
     * */
    //angriffD3();

    // Ende der Ausführung wird gespeichert
    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    /**
     * Ausgabe der Zeit die für die Berechnung benötigt wurde
     * */
    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

    cout << "\nGesamte benötigte Zeit: \n\t" << (((misec / 1000) / 1000) / 60) % 60 << ":"
         << ((misec / 1000) / 1000) % 60 << ":"
         << (misec / 1000) % 1000
         << ":" << misec % 1000 << " [m:ss:ms:μm]" << endl;

    return 0;
}
