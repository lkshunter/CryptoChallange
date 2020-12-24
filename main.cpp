#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <NTL/mat_GF2.h>
#include <algorithm>
#include <fstream>
#include <regex>

using namespace std;

/*
 * Entfernt einen bestimten Char aus einem char Pointer
 * */
void removeChar(string &str, char elements[]) {

    for (unsigned int i = 0; i < strlen(elements); ++i) {
        str.erase(std::remove(str.begin(), str.end(), elements[i]), str.end());
    }
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

        while (getline(test1, segment, '+')) {
            temp.push_back(segment);
        }
        list.push_back(temp);

    }

    /*
     * Die Summanden können aus einer Multiplikation bestehen
     * Für den Fall wird sie aufgeteilt
     *
     * Ob eine Multiplikation oder nur ein Summand sie werden in einem Vector angelegt
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
 * Verschlüsselt einen Klartext mit einem belibigen publicKey
 * */
vector<int> encrypt(vector<int> clear, vector<vector<vector<int>>> pKey) {
    vector<int> chi;
    int cipher, offset = 0;

    for (int o = 0; o < clear.size(); o += pKey.size()) {


        if (o != 0) {
            offset = o;
        }

        // Geht jede Funktion nacheinander durch
        for (int i = 0; i < pKey.size(); i++) {
            vector<vector<int>> funktion = pKey.at(i);
            cipher = 0;

            /*
             * Geht alle Multiplicationen durch und addiert die einelnen Ergebnisse
             *
             * Beispiel x_2 * x_4
             * */
            for (int j = 0; j < funktion.size(); j++) {
                vector<int> teilFunktion = funktion.at(j);

                /*
                 * Da nicht jeder Teil der Funktion aus zwei Operanden besteht muss daruf geprüft werden
                 *
                 * Beispiel x_2 * x_4 oder x_3
                 * */
                if (teilFunktion.size() == 2) {
                    cipher += clear.at(offset + teilFunktion.at(0) - 1) * clear.at(offset + teilFunktion.at(1) - 1);
                } else if (teilFunktion.size() == 1) {
                    cipher += clear.at(offset + teilFunktion.at(0) - 1);
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
 * Erzeugt aus dem Klar-Geheim Kompromiss die Stufenmatrix
 * */
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
         * Multiplikation
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

    NTL::gauss(triangle);

    cout << triangle << "\n" << endl;

    return triangle;
}

/*
 * Eurzeugt die spezielle Lösung für die Matrix
 * */
vector<NTL::mat_GF2> matrixAufrollen(NTL::mat_GF2 dummy) {
    vector<NTL::mat_GF2> spezLoes;
    NTL::mat_GF2 triMat = dummy;
    int rowMax = -1;

    int test = 0;
    for (int i = 0; i < triMat.NumRows(); i++) {

        for (int j = 0; j < triMat.NumCols(); j++) {
            if (NTL::IsZero(triMat[i][j])) {
                test++;
            }
            if (test == triMat.NumCols()) {
                rowMax = i - 1;
                break;
            }
        }

        if (rowMax != -1) {
            break;
        }
        test = 0;
    }

    if (rowMax == -1) {
        rowMax = triMat.NumRows() - 1;
    }

    if (!(NTL::IsZero(triMat))) {
        int col = triMat.NumCols();

        printf("\n");

        vector<int> expectedOne;
        vector<int> freeVar;
        vector<int> rowJump;
        expectedOne.push_back(rowMax - 1);
        expectedOne.push_back(col - 1);

        for (int i = rowMax; i >= 0; i--) {
            if (!(NTL::IsZero(triMat[i][expectedOne.at(1)]))) {
                int check = 0;
                int indexOfNull = -1;

                for (int o = expectedOne.at(1) - 1; o >= 0; o--) {
                    if (!(NTL::IsZero(triMat[i][o]))) {
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
                    if (!(NTL::IsZero(triMat[i][o]))) {
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

        cout << "Liste aller freien Variablen: " << endl;
        for (int v = 0; v < freeVar.size(); v++) {
            cout << "x_" << freeVar[v] + 1 << endl;
        }
        cout << endl;

        cout << "Liste aller Speziellen Lösungen: " << endl;
        for (int v = 0; v < freeVar.size(); v++) {
            NTL::mat_GF2 vec;
            vec.SetDims(triMat.NumCols(), 1);

            vec[freeVar[v]][0] = 1;

            int index = triMat.NumCols() - 1;
            for (int w = rowMax; w >= 0; w--) {
                int test = 0;

                for (int g = 0; g < rowJump.size(); g++) {
                    if (w == rowJump[g]) {
                        index--;
                    }
                }

                for (int g = triMat.NumCols() - 1; g >= 0; g--) {
                    if (!(NTL::IsZero(triMat[w][g]))) {
                        if (!(NTL::IsZero(vec[g][0]))) {
                            test += 1;
                        }
                    }
                }

                if ((test % 2) == 1) {
                    vec[index][0] = 1;
                }
                index--;
            }

            spezLoes.push_back(vec);
            cout << vec << "\n" << endl;
        }
    }

    return spezLoes;
}

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

void angriff(vector<vector<vector<int>>> &publicK, vector<int> &clear, vector<int> &chi) {

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

    NTL::mat_GF2 b;
    b = createBasis(specialSolution, publicK.size(), chi);

    vector<NTL::mat_GF2> c;
    c = matrixAufrollen(b);
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

    if (probe == chi) {
        cout << "Probe bestanden" << endl;
    } else {
        cout << "Probe nicht bestanden" << endl;
    }

    cout << "Lösung: ";
    for (int i = 0; i < c.at(0).NumRows(); i++) {
        cout << c.at(0)[i][0] << " ";
    }
    cout << endl;
}

/*
 * Erzeugt für eine bestimmt Matrixbreite ausreichend zufälligen Klartext
 * */
vector<int> generateRandomClear(int Range) {
    cout << Range << " = " << (2 * pow(Range, Range)) * Range << endl;
    int bound = (2 * pow(Range, 2)) * Range;
    vector<int> ran;


    for (int i = 0; i < bound; i++) {
        ran.push_back(rand() % 2);
    }

    return ran;
}

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

vector<string> readChallengeIn(string filename) {
    // Linux
    //ifstream t("/mnt/c/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);

    // Windows
    ifstream t("C:/Users/Lukas/CLionProjects/CryptoChallange/Zusatsmaterialien/Aufgabe/" + filename);


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

void angriffGruppeN(int gruppe) {

    string file = "kryptochallengegruppe";
    file += to_string(gruppe);
    file += ".txt";

    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    vector<string> input = readChallengeIn(file);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    long misec = chrono::duration_cast<chrono::microseconds>(end - start).count();

    std::cout << misec / 1000 << ":" << misec % 1000 << " [ms:μm]\n" << endl;

    vector<vector<vector<int>>> publicK = createPublicKey(input.at(0));
    vector<int> clear = generateRandomClear(publicK.size());
    vector<int> chi = createVector(input.at(1), ',');

    angriff(publicK, clear, chi);
}

void angriffDatei(string file) {

    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    vector<string> input = readChallengeIn(file);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    long misec = chrono::duration_cast<chrono::microseconds>(end - start).count();

    std::cout << misec / 1000 << ":" << misec % 1000 << " [ms:μm]\n" << endl;

    vector<vector<vector<int>>> publicK = createPublicKey(input.at(0));
    vector<int> clear = generateRandomClear(publicK.size());
    vector<int> chi = createVector(input.at(1), ',');

    angriff(publicK, clear, chi);
}

int main() {

    srand(time(0));

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();


    /*
     *
     * */
    angriffGruppeN(1);

    //angriffDatei("BeispielAngriffd7.txt");

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    cout << "\nBenötigte Zeiten: \n";

    int min = chrono::duration_cast<chrono::minutes>(end - begin).count();
    int sec = chrono::duration_cast<chrono::seconds>(end - begin).count();
    long msec = chrono::duration_cast<chrono::milliseconds>(end - begin).count();
    long misec = chrono::duration_cast<chrono::microseconds>(end - begin).count();

    std::cout << min << ":" << sec % 60 << ":" << msec % 1000 << ":" << misec % 1000 << " [m:ss:ms:μm]\n" << endl;

    return 0;
}
