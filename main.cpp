#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <NTL/mat_GF2.h>
#include <algorithm>

using namespace std;

/*
 * Entfernt einen bestimten Char aus einem char Pointer
 * */
void removeChar(char *str, char element) {
    char *dest = str;
    while (*str) {
        if (*str != element) {
            *dest++ = *str;
        }
        str++;
    }
    *dest = '\0';
}

/*
 * Erzeugt aus einem char Pointer
 * */
vector<vector<vector<int>>> createPublicKey(char *pKey) {

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
    removeChar(pKey, '\n');
    removeChar(pKey, ' ');
    removeChar(pKey, '_');
    removeChar(pKey, 'x');

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
                if (teilFunktion.size() == 1) {
                    cipher += clear.at(offset + teilFunktion.at(0) - 1);
                } else if (teilFunktion.size() == 2) {
                    cipher += clear.at(offset + teilFunktion.at(0) - 1) * clear.at(offset + teilFunktion.at(1) - 1);
                }

            }

            /*
             * Das Ergebniss der gesamten Funktion wird Modulo 2 gerechnet und in den Vector als chiText gespeichert
             * */
            cipher = cipher % 2;
            chi.push_back(cipher);
        }

    }
    return chi;
}

/*
 * Erzeugt aus einem String eine Vector mit dem Klartext
 * */
vector<int> createClearVector(string clear, char c) {
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
NTL::mat_GF2 createTriangleMatrix(vector<int> clear, vector<int> chi, int l) {
    NTL::mat_GF2 triangle;
    triangle.SetDims((clear.size() / l), l * l);
    NTL::clear(triangle);
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

                /*
                freeVar.push_back(expectedOne.at(1));
                rowJump.push_back(i);
                 */

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

        for (int v = 0; v < freeVar.size(); v++) {
            NTL::mat_GF2 vec;
            vec.SetDims(triMat.NumCols(), 1);

            vec[freeVar[v]][0] = 1;

            //cout << "Es wurde x_" << freeVar[v] + 1  << " auf Eins gesetzt\n" << vec << "\n" << endl;

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
        }
    }

    cout << "Liste aller Speziellen Lösungen: " << endl;
    for (int v = 0; v < spezLoes.size(); v++) {
        cout << spezLoes[v] << "\n" << endl;
    }
    cout << endl;

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

    //cout << ende << "\n" << endl;

    NTL::gauss(ende);

    cout << ende << "\n" << endl;

    //printf("Test");

    return ende;

}

void angriff(char pub[], string clear, char seperator, vector<int> doof) {

    /*
     * Erzeugt aus dem String den für die Berechnung benötigten Vector
     * */
    vector<vector<vector<int>>> publicK;
    publicK = createPublicKey(pub);

    vector<int> klartext;
    klartext = createClearVector(clear, seperator);

    // Erzeugen des Chitextes
    vector<int> chiText;
    chiText = encrypt(klartext, publicK);

    NTL::mat_GF2 a;
    a = createTriangleMatrix(klartext, chiText, publicK.size());

    vector<NTL::mat_GF2> specialSolution;
    specialSolution = matrixAufrollen(a);


    NTL::mat_GF2 b;
    b = createBasis(specialSolution, publicK.size(), doof);

    vector<NTL::mat_GF2> c;
    c = matrixAufrollen(b);

}

void angriff(char pub[], vector<int> clear, char seperator, vector<int> doof) {

    /*
     * Erzeugt aus dem String den für die Berechnung benötigten Vector
     * */
    vector<vector<vector<int>>> publicK;
    publicK = createPublicKey(pub);

    vector<int> klartext = clear;

    // Erzeugen des Chitextes
    vector<int> chiText;
    chiText = encrypt(klartext, publicK);

    NTL::mat_GF2 a;
    a = createTriangleMatrix(klartext, chiText, publicK.size());

    vector<NTL::mat_GF2> specialSolution;
    specialSolution = matrixAufrollen(a);


    NTL::mat_GF2 b;
    b = createBasis(specialSolution, publicK.size(), doof);

    vector<NTL::mat_GF2> c;
    c = matrixAufrollen(b);

    cout << "" <<endl;
}

/*
 * Erzeugt für eine bestimmt Matrixbreite ausreichend zufälligen Klartext
 * */
vector<int> generateRandomClear(int n, int Range) {
    int bound = n * (Range * Range);
    vector<int> ran;

    for (int i = 0; i < bound; i++) {
        ran.push_back(rand() % 2);
    }

    return ran;
}

void angriffD3() {
    char publicKey[] = "x_1*x_3 + x_2*x_3 + x_2,\n"
                       "    x_1*x_3 + x_1 + x_2 + x_3,\n"
                       "    x_1*x_2 + x_3";

    string clear = "0 1 1 0 0 0 1 0 1 1 0 0 0 0 1 0 1 0 1 1 0 1 1 1 1 0 1 0 1 1 1 0 0 0 1 0 1 0 1 0 1 1 1 1 1 0 0 0 0 0 1 0 1 0";

    vector<int> chi = {1, 1, 1};

    angriff(publicKey, clear, ' ', chi);
}

void angriffD5() {
    char publicKey[] = "x_1*x_3 + x_2*x_3 + x_1*x_5 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_1,\n"
                       "    x_1*x_2 + x_1*x_3 + x_2*x_3 + x_3*x_4 + x_1*x_5 + x_4*x_5 + x_2 + x_3,\n"
                       "    x_1*x_2 + x_1*x_3 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_5 + x_1 + \n"
                       "        x_2 + x_3,\n"
                       "    x_1*x_3 + x_2*x_3 + x_2*x_4 + x_4*x_5 + x_2 + x_4,\n"
                       "    x_1*x_2 + x_1*x_4 + x_3*x_4 + x_4*x_5 + x_1 + x_2 + x_5";

    string clear = "0 1 1 1 0 1 0 1 0 0 0 0 1 1 1 1 1 0 1 0 0 1 1 1 1 0 0 1 1 1 1 0 1 1 0 1 1 1 1 0 1 1 1 1 0 1 1 1 0 0 1 1 1 1 1 1 0 1 0 1 0 1 1 1 0 1 0 0 0 1 0 1 1 0 0 0 1 0 1 1 1 0 1 1 1 0 0 0 0 0 0 0 0 1 1 1 0 0 0 0 1 0 0 1 1 0 0 0 0 0 0 1 1 0 1 1 0 1 0 0 1 0 1 1 1 0 1 1 1 0 1 0 1 1 1 0 0 0 0 0 1 1 0 1 0 0 0 1 1 0 1 0 0 1 0 0 0 0 0 0 1 0 0 0 1 0 1 0 1 0 0 1 1 0 0 0 1 0 0 0 1 0 1 0 0 1 1 0 0 0 1 1 1 1 0 0 0 1 1 1 0 1 1 1 0 1 1 1 0 1 1 1 0 0 1 1 1 1 1 0 1 1 1 0 1 1 0 1 0 1 0 1 1 1 0 1 0 0 1 0 1 0 0 0 0 1 0 1 1 1";

    vector<int> chi = {1, 0, 0, 0, 1};

    angriff(publicKey, clear, ' ', chi);
}

void angriffD7() {
    char publicKey[] = "x_1*x_2 + x_2*x_3 + x_2*x_4 + x_3*x_4 + x_3*x_5 + x_1*x_6 + x_3*x_6 + \n"
                       "        x_4*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_3*x_7 + x_6,\n"
                       "    x_1*x_2 + x_2*x_5 + x_4*x_5 + x_2*x_6 + x_3*x_6 + x_5*x_6 + x_6*x_7 + x_2 + \n"
                       "        x_3 + x_4 + x_5,\n"
                       "    x_2*x_3 + x_3*x_4 + x_2*x_6 + x_5*x_6 + x_2*x_7 + x_3*x_7 + x_6*x_7 + x_1 + \n"
                       "        x_2 + x_4 + x_6 + x_7,\n"
                       "    x_1*x_2 + x_2*x_3 + x_2*x_5 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + \n"
                       "        x_4*x_6 + x_5*x_6 + x_2 + x_3 + x_4 + x_7,\n"
                       "    x_1*x_4 + x_3*x_4 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_2*x_6 + x_4*x_6 + \n"
                       "        x_5*x_6 + x_1*x_7 + x_3*x_7 + x_5*x_7 + x_6*x_7 + x_1 + x_2 + x_3 + x_4,\n"
                       "    x_1*x_3 + x_2*x_4 + x_1*x_5 + x_4*x_5 + x_1*x_6 + x_3*x_6 + x_1*x_7 + \n"
                       "        x_2*x_7 + x_3*x_7 + x_4*x_7 + x_1 + x_2 + x_3 + x_4 + x_5 + x_7,\n"
                       "    x_1*x_3 + x_1*x_4 + x_3*x_5 + x_4*x_5 + x_2*x_6 + x_1*x_7 + x_4*x_7 + \n"
                       "        x_5*x_7 + x_2 + x_5 + x_7";

    vector<int> clear = generateRandomClear(6,7);

    vector<int> chi = {1, 0, 1, 1, 0, 1, 1};

    angriff(publicKey, clear, ' ', chi);
}

void angriffGruppe1() {
    char publicKey[] = "x_1*x_2 + x_1*x_3 + x_2*x_3 + x_2*x_4 + x_3*x_4 + x_2*x_6 + x_3*x_6 + x_4*x_6 + x_1*x_7 + x_3*x_7 + x_4*x_7 + x_5*x_7 + x_2*x_8 + x_3*x_8 + x_4*x_8 + x_5*x_8 + x_6*x_8 + x_1*x_9 + x_3*x_9 + x_7*x_9 + \n"
                       "        x_8*x_9 + x_2*x_10 + x_6*x_10 + x_8*x_10 + x_6*x_11 + x_7*x_11 + x_9*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_7*x_12 + x_8*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_5*x_13 \n"
                       "        + x_6*x_13 + x_7*x_13 + x_8*x_13 + x_9*x_13 + x_11*x_13 + x_2*x_14 + x_3*x_14 + x_5*x_14 + x_7*x_14 + x_8*x_14 + x_9*x_14 + x_10*x_14 + x_11*x_14 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_8*x_15 + \n"
                       "        x_10*x_15 + x_2*x_16 + x_7*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_13*x_16 + x_14*x_16 + x_2*x_17 + x_3*x_17 + x_6*x_17 + x_7*x_17 + x_9*x_17 + x_14*x_17 + x_15*x_17 + x_2*x_18 + x_3*x_18 + \n"
                       "        x_8*x_18 + x_9*x_18 + x_11*x_18 + x_12*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + x_17*x_18 + x_2*x_19 + x_5*x_19 + x_8*x_19 + x_9*x_19 + x_16*x_19 + x_18*x_19 + x_4*x_20 + x_5*x_20 + x_6*x_20 + \n"
                       "        x_7*x_20 + x_11*x_20 + x_12*x_20 + x_14*x_20 + x_18*x_20 + x_1*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + \n"
                       "        x_16*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_2*x_22 + x_4*x_22 + x_9*x_22 + x_10*x_22 + x_13*x_22 + x_16*x_22 + x_18*x_22 + x_21*x_22 + x_6*x_23 + x_7*x_23 + x_8*x_23 + x_11*x_23 + x_17*x_23 \n"
                       "        + x_18*x_23 + x_21*x_23 + x_22*x_23 + x_1*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_12*x_24 + x_14*x_24 + x_17*x_24 + x_21*x_24 + x_22*x_24 + x_8*x_25\n"
                       "        + x_9*x_25 + x_12*x_25 + x_14*x_25 + x_15*x_25 + x_16*x_25 + x_17*x_25 + x_20*x_25 + x_23*x_25 + x_24*x_25 + x_3*x_26 + x_5*x_26 + x_6*x_26 + x_8*x_26 + x_11*x_26 + x_12*x_26 + x_14*x_26 + \n"
                       "        x_15*x_26 + x_17*x_26 + x_23*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_4*x_27 + x_5*x_27 + x_6*x_27 + x_9*x_27 + x_11*x_27 + x_13*x_27 + x_16*x_27 + x_17*x_27 + x_18*x_27 + x_19*x_27 + x_20*x_27\n"
                       "        + x_22*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_6*x_28 + x_7*x_28 + x_8*x_28 + x_10*x_28 + x_13*x_28 + x_14*x_28 + x_20*x_28 + x_21*x_28 + x_1*x_29 + x_4*x_29 + x_5*x_29 + x_6*x_29 + \n"
                       "        x_7*x_29 + x_9*x_29 + x_11*x_29 + x_18*x_29 + x_20*x_29 + x_23*x_29 + x_24*x_29 + x_28*x_29 + x_1*x_30 + x_3*x_30 + x_4*x_30 + x_7*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + x_19*x_30 + x_21*x_30 \n"
                       "        + x_22*x_30 + x_24*x_30 + x_26*x_30 + x_27*x_30 + x_28*x_30 + x_1*x_31 + x_2*x_31 + x_11*x_31 + x_13*x_31 + x_20*x_31 + x_21*x_31 + x_23*x_31 + x_26*x_31 + x_29*x_31 + x_10*x_32 + x_12*x_32 + \n"
                       "        x_13*x_32 + x_18*x_32 + x_21*x_32 + x_22*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_1*x_33 + x_2*x_33 + x_5*x_33 + x_7*x_33 + x_8*x_33 + x_10*x_33 + x_11*x_33 + x_14*x_33 + x_15*x_33\n"
                       "        + x_16*x_33 + x_18*x_33 + x_19*x_33 + x_21*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_2*x_34 + x_3*x_34 + x_5*x_34 + x_7*x_34 + x_8*x_34 + x_9*x_34 + x_10*x_34 + x_15*x_34 + x_18*x_34 + \n"
                       "        x_19*x_34 + x_24*x_34 + x_25*x_34 + x_27*x_34 + x_28*x_34 + x_30*x_34 + x_33*x_34 + x_3*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_14*x_35 + x_15*x_35 + x_16*x_35\n"
                       "        + x_18*x_35 + x_19*x_35 + x_20*x_35 + x_22*x_35 + x_23*x_35 + x_25*x_35 + x_26*x_35 + x_28*x_35 + x_30*x_35 + x_31*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_6*x_36 + x_8*x_36\n"
                       "        + x_9*x_36 + x_11*x_36 + x_15*x_36 + x_18*x_36 + x_19*x_36 + x_21*x_36 + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_29*x_36 + x_31*x_36 + x_32*x_36 + x_34*x_36 + x_35*x_36 + x_2*x_37 + x_4*x_37 + \n"
                       "        x_5*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + x_12*x_37 + x_17*x_37 + x_20*x_37 + x_22*x_37 + x_25*x_37 + x_28*x_37 + x_29*x_37 + x_31*x_37 + x_33*x_37 + x_34*x_37 + x_36*x_37 + x_1*x_38 + x_3*x_38 \n"
                       "        + x_7*x_38 + x_8*x_38 + x_10*x_38 + x_14*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_19*x_38 + x_21*x_38 + x_23*x_38 + x_28*x_38 + x_30*x_38 + x_32*x_38 + x_33*x_38 + x_36*x_38 + x_37*x_38 + \n"
                       "        x_2*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_8*x_39 + x_10*x_39 + x_11*x_39 + x_12*x_39 + x_13*x_39 + x_15*x_39 + x_16*x_39 + x_17*x_39 + x_20*x_39 + x_21*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39\n"
                       "        + x_26*x_39 + x_27*x_39 + x_28*x_39 + x_30*x_39 + x_31*x_39 + x_33*x_39 + x_34*x_39 + x_35*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_7*x_40 + x_9*x_40 + x_10*x_40 + x_12*x_40 + x_13*x_40 + \n"
                       "        x_15*x_40 + x_16*x_40 + x_18*x_40 + x_19*x_40 + x_20*x_40 + x_22*x_40 + x_23*x_40 + x_25*x_40 + x_26*x_40 + x_27*x_40 + x_32*x_40 + x_34*x_40 + x_38*x_40 + x_1*x_41 + x_4*x_41 + x_7*x_41 + \n"
                       "        x_14*x_41 + x_15*x_41 + x_17*x_41 + x_18*x_41 + x_21*x_41 + x_22*x_41 + x_25*x_41 + x_26*x_41 + x_27*x_41 + x_29*x_41 + x_33*x_41 + x_35*x_41 + x_37*x_41 + x_38*x_41 + x_39*x_41 + x_40*x_41 + \n"
                       "        x_3*x_42 + x_7*x_42 + x_10*x_42 + x_11*x_42 + x_15*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_21*x_42 + x_22*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_28*x_42 + x_29*x_42 + \n"
                       "        x_31*x_42 + x_32*x_42 + x_33*x_42 + x_36*x_42 + x_39*x_42 + x_41*x_42 + x_2*x_43 + x_3*x_43 + x_6*x_43 + x_7*x_43 + x_8*x_43 + x_10*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_17*x_43 + x_18*x_43\n"
                       "        + x_19*x_43 + x_21*x_43 + x_22*x_43 + x_23*x_43 + x_27*x_43 + x_31*x_43 + x_33*x_43 + x_37*x_43 + x_38*x_43 + x_40*x_43 + x_41*x_43 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_8*x_44 + x_10*x_44 + \n"
                       "        x_14*x_44 + x_16*x_44 + x_18*x_44 + x_19*x_44 + x_22*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_32*x_44 + x_34*x_44 + x_35*x_44 + x_38*x_44 + x_39*x_44 + x_41*x_44 + x_42*x_44 + x_1*x_45 + \n"
                       "        x_4*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_9*x_45 + x_10*x_45 + x_13*x_45 + x_14*x_45 + x_17*x_45 + x_18*x_45 + x_20*x_45 + x_21*x_45 + x_23*x_45 + x_26*x_45 + x_27*x_45 + x_31*x_45 \n"
                       "        + x_33*x_45 + x_34*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_6*x_46 + x_7*x_46 + x_8*x_46 + x_9*x_46 + x_11*x_46 + x_13*x_46 + \n"
                       "        x_14*x_46 + x_17*x_46 + x_18*x_46 + x_22*x_46 + x_28*x_46 + x_30*x_46 + x_31*x_46 + x_33*x_46 + x_35*x_46 + x_37*x_46 + x_39*x_46 + x_42*x_46 + x_43*x_46 + x_1*x_47 + x_2*x_47 + x_4*x_47 + \n"
                       "        x_11*x_47 + x_12*x_47 + x_14*x_47 + x_17*x_47 + x_21*x_47 + x_22*x_47 + x_24*x_47 + x_25*x_47 + x_26*x_47 + x_29*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + \n"
                       "        x_43*x_47 + x_1*x_48 + x_2*x_48 + x_4*x_48 + x_5*x_48 + x_6*x_48 + x_7*x_48 + x_9*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_17*x_48 + x_19*x_48 + x_23*x_48 + x_26*x_48 + x_27*x_48 +\n"
                       "        x_29*x_48 + x_30*x_48 + x_33*x_48 + x_34*x_48 + x_35*x_48 + x_38*x_48 + x_39*x_48 + x_40*x_48 + x_43*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_1*x_49 + x_2*x_49 + x_3*x_49 + x_4*x_49 + x_5*x_49\n"
                       "        + x_7*x_49 + x_8*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 + x_13*x_49 + x_15*x_49 + x_17*x_49 + x_19*x_49 + x_22*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + \n"
                       "        x_31*x_49 + x_33*x_49 + x_34*x_49 + x_35*x_49 + x_37*x_49 + x_39*x_49 + x_40*x_49 + x_41*x_49 + x_42*x_49 + x_46*x_49 + x_1 + x_2 + x_3 + x_6 + x_7 + x_11 + x_12 + x_13 + x_14 + x_17 + x_18 + x_19\n"
                       "        + x_21 + x_22 + x_26 + x_28 + x_33 + x_36 + x_39 + x_40 + x_41 + x_43 + x_45 + x_47,\n"
                       "    x_2*x_3 + x_2*x_5 + x_3*x_6 + x_3*x_7 + x_5*x_7 + x_2*x_8 + x_5*x_8 + x_3*x_9 + x_4*x_9 + x_5*x_9 + x_7*x_9 + x_1*x_10 + x_2*x_10 + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_3*x_11 + x_4*x_11 + \n"
                       "        x_6*x_11 + x_7*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_9*x_12 + x_11*x_12 + x_1*x_13 + x_4*x_13 + x_6*x_13 + x_8*x_13 + x_2*x_14 + x_3*x_14 + x_4*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14\n"
                       "        + x_11*x_14 + x_13*x_14 + x_1*x_15 + x_3*x_15 + x_6*x_15 + x_8*x_15 + x_12*x_15 + x_14*x_15 + x_1*x_16 + x_4*x_16 + x_6*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_11*x_16 + x_2*x_17 + x_3*x_17 + \n"
                       "        x_6*x_17 + x_8*x_17 + x_10*x_17 + x_13*x_17 + x_1*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_7*x_18 + x_10*x_18 + x_11*x_18 + x_15*x_18 + x_2*x_19 + x_5*x_19 + x_6*x_19 + x_8*x_19 + x_10*x_19 + \n"
                       "        x_11*x_19 + x_12*x_19 + x_17*x_19 + x_3*x_20 + x_4*x_20 + x_5*x_20 + x_7*x_20 + x_8*x_20 + x_9*x_20 + x_10*x_20 + x_11*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 + x_19*x_20 + x_1*x_21 + x_2*x_21 + \n"
                       "        x_3*x_21 + x_4*x_21 + x_5*x_21 + x_8*x_21 + x_10*x_21 + x_13*x_21 + x_19*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_6*x_22 + x_8*x_22 + x_9*x_22 + x_10*x_22 + x_11*x_22 + x_13*x_22 + x_17*x_22 + \n"
                       "        x_18*x_22 + x_19*x_22 + x_1*x_23 + x_2*x_23 + x_7*x_23 + x_9*x_23 + x_10*x_23 + x_12*x_23 + x_15*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_21*x_23 + x_22*x_23 + x_1*x_24 + x_2*x_24 + x_6*x_24 +\n"
                       "        x_10*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_15*x_24 + x_16*x_24 + x_18*x_24 + x_19*x_24 + x_1*x_25 + x_2*x_25 + x_6*x_25 + x_7*x_25 + x_8*x_25 + x_9*x_25 + x_11*x_25 + x_13*x_25 + x_16*x_25 \n"
                       "        + x_17*x_25 + x_18*x_25 + x_22*x_25 + x_23*x_25 + x_4*x_26 + x_5*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_10*x_26 + x_16*x_26 + x_17*x_26 + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_1*x_27\n"
                       "        + x_3*x_27 + x_5*x_27 + x_7*x_27 + x_8*x_27 + x_11*x_27 + x_17*x_27 + x_18*x_27 + x_19*x_27 + x_21*x_27 + x_23*x_27 + x_25*x_27 + x_26*x_27 + x_2*x_28 + x_3*x_28 + x_6*x_28 + x_8*x_28 + x_9*x_28 +\n"
                       "        x_10*x_28 + x_12*x_28 + x_14*x_28 + x_16*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_26*x_28 + x_27*x_28 + x_2*x_29 + x_4*x_29 + x_6*x_29 + x_7*x_29 + x_8*x_29 + x_11*x_29 + x_12*x_29 + x_14*x_29\n"
                       "        + x_15*x_29 + x_16*x_29 + x_18*x_29 + x_19*x_29 + x_23*x_29 + x_26*x_29 + x_28*x_29 + x_2*x_30 + x_4*x_30 + x_5*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_16*x_30 + x_17*x_30 + x_24*x_30\n"
                       "        + x_27*x_30 + x_2*x_31 + x_5*x_31 + x_6*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 + x_16*x_31 + x_20*x_31 + x_22*x_31 + x_24*x_31 + x_25*x_31 + x_29*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32\n"
                       "        + x_3*x_32 + x_5*x_32 + x_8*x_32 + x_11*x_32 + x_12*x_32 + x_16*x_32 + x_17*x_32 + x_23*x_32 + x_26*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_1*x_33 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_5*x_33 \n"
                       "        + x_7*x_33 + x_8*x_33 + x_10*x_33 + x_12*x_33 + x_15*x_33 + x_17*x_33 + x_18*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_31*x_33 + x_32*x_33 + x_3*x_34 + \n"
                       "        x_4*x_34 + x_6*x_34 + x_8*x_34 + x_10*x_34 + x_12*x_34 + x_13*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_21*x_34 + x_22*x_34 + x_23*x_34 + x_25*x_34 + x_28*x_34 + \n"
                       "        x_30*x_34 + x_31*x_34 + x_33*x_34 + x_1*x_35 + x_4*x_35 + x_6*x_35 + x_11*x_35 + x_14*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_19*x_35 + x_22*x_35 + x_23*x_35 + x_26*x_35 + x_28*x_35 + \n"
                       "        x_29*x_35 + x_30*x_35 + x_31*x_35 + x_1*x_36 + x_4*x_36 + x_6*x_36 + x_7*x_36 + x_9*x_36 + x_12*x_36 + x_15*x_36 + x_17*x_36 + x_18*x_36 + x_20*x_36 + x_22*x_36 + x_23*x_36 + x_25*x_36 + x_27*x_36\n"
                       "        + x_28*x_36 + x_4*x_37 + x_5*x_37 + x_6*x_37 + x_11*x_37 + x_15*x_37 + x_16*x_37 + x_17*x_37 + x_18*x_37 + x_19*x_37 + x_21*x_37 + x_23*x_37 + x_31*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + \n"
                       "        x_35*x_37 + x_36*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_5*x_38 + x_8*x_38 + x_9*x_38 + x_10*x_38 + x_11*x_38 + x_13*x_38 + x_19*x_38 + x_20*x_38 + x_23*x_38 + x_24*x_38 + x_25*x_38 + x_26*x_38 \n"
                       "        + x_27*x_38 + x_29*x_38 + x_31*x_38 + x_34*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + x_4*x_39 + x_7*x_39 + x_9*x_39 + x_10*x_39 + x_12*x_39 + x_13*x_39 + x_14*x_39 + x_15*x_39 + x_18*x_39 + \n"
                       "        x_19*x_39 + x_20*x_39 + x_24*x_39 + x_27*x_39 + x_28*x_39 + x_33*x_39 + x_35*x_39 + x_36*x_39 + x_37*x_39 + x_38*x_39 + x_2*x_40 + x_6*x_40 + x_8*x_40 + x_11*x_40 + x_12*x_40 + x_14*x_40 + \n"
                       "        x_18*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_33*x_40 + x_34*x_40 + x_35*x_40 + x_36*x_40 + x_37*x_40 + x_38*x_40 + x_39*x_40 + x_1*x_41 + x_2*x_41 + x_4*x_41 + x_7*x_41 + x_8*x_41\n"
                       "        + x_12*x_41 + x_13*x_41 + x_15*x_41 + x_16*x_41 + x_18*x_41 + x_19*x_41 + x_25*x_41 + x_26*x_41 + x_29*x_41 + x_30*x_41 + x_36*x_41 + x_37*x_41 + x_38*x_41 + x_39*x_41 + x_1*x_42 + x_3*x_42 + \n"
                       "        x_4*x_42 + x_5*x_42 + x_8*x_42 + x_12*x_42 + x_13*x_42 + x_15*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_20*x_42 + x_21*x_42 + x_24*x_42 + x_25*x_42 + x_28*x_42 + x_31*x_42 + x_35*x_42 + \n"
                       "        x_36*x_42 + x_41*x_42 + x_1*x_43 + x_4*x_43 + x_6*x_43 + x_9*x_43 + x_11*x_43 + x_12*x_43 + x_17*x_43 + x_20*x_43 + x_21*x_43 + x_22*x_43 + x_26*x_43 + x_27*x_43 + x_28*x_43 + x_31*x_43 + \n"
                       "        x_36*x_43 + x_38*x_43 + x_39*x_43 + x_41*x_43 + x_42*x_43 + x_2*x_44 + x_4*x_44 + x_5*x_44 + x_6*x_44 + x_10*x_44 + x_11*x_44 + x_13*x_44 + x_23*x_44 + x_27*x_44 + x_28*x_44 + x_32*x_44 + \n"
                       "        x_33*x_44 + x_36*x_44 + x_37*x_44 + x_38*x_44 + x_2*x_45 + x_3*x_45 + x_8*x_45 + x_9*x_45 + x_11*x_45 + x_13*x_45 + x_14*x_45 + x_19*x_45 + x_20*x_45 + x_24*x_45 + x_25*x_45 + x_27*x_45 + \n"
                       "        x_29*x_45 + x_31*x_45 + x_33*x_45 + x_34*x_45 + x_36*x_45 + x_37*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_8*x_46 + x_11*x_46 + x_13*x_46 + x_17*x_46 + x_19*x_46 + \n"
                       "        x_20*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_27*x_46 + x_28*x_46 + x_30*x_46 + x_31*x_46 + x_35*x_46 + x_36*x_46 + x_37*x_46 + x_39*x_46 + x_40*x_46 + x_41*x_46 + x_44*x_46 + x_4*x_47 + \n"
                       "        x_5*x_47 + x_6*x_47 + x_7*x_47 + x_8*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_17*x_47 + x_20*x_47 + x_22*x_47 + x_24*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + \n"
                       "        x_41*x_47 + x_46*x_47 + x_1*x_48 + x_2*x_48 + x_3*x_48 + x_5*x_48 + x_7*x_48 + x_9*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_14*x_48 + x_16*x_48 + x_17*x_48 + x_18*x_48 + x_20*x_48 + x_22*x_48 \n"
                       "        + x_23*x_48 + x_24*x_48 + x_26*x_48 + x_27*x_48 + x_28*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_34*x_48 + x_36*x_48 + x_38*x_48 + x_39*x_48 + x_41*x_48 + x_43*x_48 + x_44*x_48 + x_1*x_49 + \n"
                       "        x_2*x_49 + x_3*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_11*x_49 + x_14*x_49 + x_16*x_49 + x_17*x_49 + x_19*x_49 + x_20*x_49 + x_27*x_49 + x_28*x_49 + x_30*x_49 + x_31*x_49 + x_34*x_49 + x_35*x_49\n"
                       "        + x_37*x_49 + x_42*x_49 + x_43*x_49 + x_44*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_3 + x_5 + x_11 + x_12 + x_13 + x_14 + x_18 + x_22 + x_23 + x_24 + x_25 + x_27 + x_28 + x_29 + \n"
                       "        x_31 + x_33 + x_34 + x_35 + x_36 + x_38 + x_39 + x_42 + x_47 + x_49,\n"
                       "    x_2*x_4 + x_3*x_4 + x_3*x_6 + x_4*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_4*x_7 + x_6*x_8 + x_1*x_9 + x_2*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_8*x_9 + x_2*x_10 + x_6*x_10 + x_7*x_10 + x_1*x_11 + \n"
                       "        x_2*x_11 + x_5*x_11 + x_8*x_11 + x_9*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_8*x_12 + x_10*x_12 + x_11*x_12 + x_3*x_13 + x_4*x_13 + x_5*x_13 + x_8*x_13 + x_9*x_13 + \n"
                       "        x_1*x_14 + x_3*x_14 + x_5*x_14 + x_8*x_14 + x_10*x_14 + x_11*x_14 + x_13*x_14 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_11*x_15 + x_12*x_15 + x_1*x_16 + x_2*x_16 + x_5*x_16 + \n"
                       "        x_6*x_16 + x_7*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_1*x_17 + x_6*x_17 + x_7*x_17 + x_8*x_17 + x_9*x_17 + x_10*x_17 + x_11*x_17 + x_13*x_17 + x_16*x_17 + x_1*x_18 + x_4*x_18 + x_8*x_18 + \n"
                       "        x_11*x_18 + x_12*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + x_5*x_19 + x_6*x_19 + x_8*x_19 + x_9*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_16*x_19 + x_2*x_20 + x_7*x_20 + x_8*x_20 + \n"
                       "        x_13*x_20 + x_15*x_20 + x_2*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_8*x_21 + x_13*x_21 + x_14*x_21 + x_15*x_21 + x_16*x_21 + x_18*x_21 + x_20*x_21 + x_4*x_22 + x_6*x_22 + x_7*x_22 + \n"
                       "        x_8*x_22 + x_9*x_22 + x_13*x_22 + x_15*x_22 + x_17*x_22 + x_18*x_22 + x_19*x_22 + x_21*x_22 + x_4*x_23 + x_6*x_23 + x_8*x_23 + x_9*x_23 + x_13*x_23 + x_17*x_23 + x_19*x_23 + x_21*x_23 + x_2*x_24 +\n"
                       "        x_3*x_24 + x_6*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_16*x_24 + x_17*x_24 + x_19*x_24 + x_20*x_24 + x_22*x_24 + x_23*x_24 + x_2*x_25 + x_3*x_25 + x_5*x_25 + x_6*x_25 + x_10*x_25 + x_11*x_25 + \n"
                       "        x_12*x_25 + x_14*x_25 + x_15*x_25 + x_16*x_25 + x_18*x_25 + x_19*x_25 + x_22*x_25 + x_23*x_25 + x_3*x_26 + x_5*x_26 + x_7*x_26 + x_10*x_26 + x_16*x_26 + x_19*x_26 + x_22*x_26 + x_23*x_26 + \n"
                       "        x_24*x_26 + x_25*x_26 + x_1*x_27 + x_6*x_27 + x_7*x_27 + x_10*x_27 + x_13*x_27 + x_16*x_27 + x_19*x_27 + x_21*x_27 + x_22*x_27 + x_24*x_27 + x_26*x_27 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_7*x_28 +\n"
                       "        x_8*x_28 + x_10*x_28 + x_12*x_28 + x_16*x_28 + x_23*x_28 + x_27*x_28 + x_3*x_29 + x_4*x_29 + x_6*x_29 + x_7*x_29 + x_8*x_29 + x_12*x_29 + x_13*x_29 + x_15*x_29 + x_16*x_29 + x_17*x_29 + x_19*x_29 \n"
                       "        + x_21*x_29 + x_24*x_29 + x_25*x_29 + x_28*x_29 + x_1*x_30 + x_3*x_30 + x_5*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_12*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_17*x_30 + x_21*x_30\n"
                       "        + x_22*x_30 + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_27*x_30 + x_2*x_31 + x_6*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_12*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_18*x_31 + x_20*x_31 + \n"
                       "        x_21*x_31 + x_22*x_31 + x_24*x_31 + x_27*x_31 + x_3*x_32 + x_5*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_14*x_32 + x_15*x_32 + x_18*x_32 + x_22*x_32 + x_24*x_32 + x_27*x_32 + x_28*x_32 + \n"
                       "        x_29*x_32 + x_31*x_32 + x_2*x_33 + x_6*x_33 + x_8*x_33 + x_9*x_33 + x_10*x_33 + x_11*x_33 + x_12*x_33 + x_13*x_33 + x_15*x_33 + x_16*x_33 + x_19*x_33 + x_23*x_33 + x_24*x_33 + x_28*x_33 + \n"
                       "        x_29*x_33 + x_32*x_33 + x_2*x_34 + x_3*x_34 + x_5*x_34 + x_7*x_34 + x_8*x_34 + x_9*x_34 + x_11*x_34 + x_12*x_34 + x_14*x_34 + x_18*x_34 + x_19*x_34 + x_23*x_34 + x_24*x_34 + x_25*x_34 + x_27*x_34 \n"
                       "        + x_32*x_34 + x_2*x_35 + x_5*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_10*x_35 + x_12*x_35 + x_16*x_35 + x_19*x_35 + x_20*x_35 + x_22*x_35 + x_23*x_35 + x_24*x_35 + x_25*x_35 + x_27*x_35 + \n"
                       "        x_30*x_35 + x_31*x_35 + x_34*x_35 + x_1*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 + x_9*x_36 + x_10*x_36 + x_12*x_36 + x_14*x_36 + x_17*x_36 + x_18*x_36 + x_20*x_36 + x_21*x_36 + x_23*x_36 + x_24*x_36\n"
                       "        + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_30*x_36 + x_34*x_36 + x_35*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_7*x_37 + x_10*x_37 + x_12*x_37 + x_14*x_37 + x_15*x_37 + x_16*x_37 + \n"
                       "        x_18*x_37 + x_19*x_37 + x_20*x_37 + x_26*x_37 + x_28*x_37 + x_30*x_37 + x_31*x_37 + x_32*x_37 + x_33*x_37 + x_36*x_37 + x_1*x_38 + x_5*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_11*x_38 + x_12*x_38\n"
                       "        + x_13*x_38 + x_14*x_38 + x_16*x_38 + x_19*x_38 + x_21*x_38 + x_22*x_38 + x_25*x_38 + x_26*x_38 + x_27*x_38 + x_29*x_38 + x_30*x_38 + x_32*x_38 + x_34*x_38 + x_35*x_38 + x_36*x_38 + x_2*x_39 + \n"
                       "        x_6*x_39 + x_9*x_39 + x_11*x_39 + x_15*x_39 + x_17*x_39 + x_18*x_39 + x_19*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_23*x_39 + x_25*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_34*x_39 + \n"
                       "        x_37*x_39 + x_38*x_39 + x_6*x_40 + x_13*x_40 + x_17*x_40 + x_18*x_40 + x_22*x_40 + x_23*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + x_38*x_40 + x_1*x_41 + x_3*x_41 + \n"
                       "        x_4*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_17*x_41 + x_20*x_41 + x_24*x_41 + x_25*x_41 + x_27*x_41 + x_28*x_41 + x_30*x_41 + x_32*x_41 + \n"
                       "        x_33*x_41 + x_34*x_41 + x_37*x_41 + x_40*x_41 + x_4*x_42 + x_6*x_42 + x_9*x_42 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_14*x_42 + x_17*x_42 + x_18*x_42 + x_22*x_42 + x_24*x_42 + x_25*x_42 + \n"
                       "        x_26*x_42 + x_28*x_42 + x_30*x_42 + x_31*x_42 + x_35*x_42 + x_36*x_42 + x_37*x_42 + x_38*x_42 + x_40*x_42 + x_2*x_43 + x_3*x_43 + x_8*x_43 + x_9*x_43 + x_10*x_43 + x_11*x_43 + x_13*x_43 + \n"
                       "        x_14*x_43 + x_16*x_43 + x_17*x_43 + x_25*x_43 + x_27*x_43 + x_29*x_43 + x_31*x_43 + x_32*x_43 + x_35*x_43 + x_37*x_43 + x_38*x_43 + x_41*x_43 + x_1*x_44 + x_2*x_44 + x_3*x_44 + x_6*x_44 + x_7*x_44\n"
                       "        + x_8*x_44 + x_10*x_44 + x_11*x_44 + x_14*x_44 + x_16*x_44 + x_17*x_44 + x_18*x_44 + x_19*x_44 + x_20*x_44 + x_23*x_44 + x_25*x_44 + x_31*x_44 + x_32*x_44 + x_37*x_44 + x_3*x_45 + x_4*x_45 + \n"
                       "        x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_15*x_45 + x_17*x_45 + x_20*x_45 + x_22*x_45 + x_24*x_45 + x_25*x_45 + x_26*x_45 + x_27*x_45 + x_28*x_45 + \n"
                       "        x_32*x_45 + x_34*x_45 + x_35*x_45 + x_36*x_45 + x_40*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_3*x_46 + x_7*x_46 + x_9*x_46 + x_10*x_46 + x_13*x_46 + x_14*x_46 + x_18*x_46 + x_20*x_46 + x_24*x_46\n"
                       "        + x_25*x_46 + x_26*x_46 + x_27*x_46 + x_29*x_46 + x_31*x_46 + x_32*x_46 + x_40*x_46 + x_41*x_46 + x_44*x_46 + x_45*x_46 + x_6*x_47 + x_7*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + \n"
                       "        x_17*x_47 + x_18*x_47 + x_22*x_47 + x_26*x_47 + x_28*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_33*x_47 + x_37*x_47 + x_38*x_47 + x_39*x_47 + x_45*x_47 + x_46*x_47 + x_2*x_48 + x_4*x_48 + \n"
                       "        x_6*x_48 + x_7*x_48 + x_9*x_48 + x_10*x_48 + x_11*x_48 + x_13*x_48 + x_14*x_48 + x_16*x_48 + x_17*x_48 + x_18*x_48 + x_20*x_48 + x_21*x_48 + x_24*x_48 + x_29*x_48 + x_31*x_48 + x_34*x_48 + \n"
                       "        x_35*x_48 + x_36*x_48 + x_37*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_45*x_48 + x_1*x_49 + x_5*x_49 + x_6*x_49 + x_14*x_49 + x_16*x_49 + x_17*x_49 + x_21*x_49 + x_22*x_49 + x_26*x_49 + \n"
                       "        x_27*x_49 + x_28*x_49 + x_30*x_49 + x_33*x_49 + x_35*x_49 + x_37*x_49 + x_40*x_49 + x_41*x_49 + x_42*x_49 + x_44*x_49 + x_4 + x_6 + x_7 + x_8 + x_9 + x_13 + x_16 + x_18 + x_19 + x_21 + x_22 + x_23\n"
                       "        + x_28 + x_32 + x_35 + x_37 + x_38 + x_39 + x_40 + x_41 + x_42 + x_48 + x_49,\n"
                       "    x_1*x_3 + x_2*x_4 + x_1*x_5 + x_2*x_5 + x_4*x_6 + x_1*x_7 + x_3*x_7 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_5*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_8*x_9 + x_1*x_10 + x_2*x_10 + x_3*x_10 + x_5*x_10 + \n"
                       "        x_6*x_10 + x_1*x_11 + x_2*x_11 + x_5*x_11 + x_10*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_11*x_12 + x_1*x_13 + x_3*x_13 + x_4*x_13 + x_7*x_13 + x_10*x_13 + x_11*x_13 + x_1*x_14 + x_2*x_14 + \n"
                       "        x_4*x_14 + x_5*x_14 + x_8*x_14 + x_13*x_14 + x_1*x_15 + x_4*x_15 + x_5*x_15 + x_8*x_15 + x_10*x_15 + x_14*x_15 + x_1*x_16 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_9*x_16 + x_14*x_16 + x_15*x_16 + \n"
                       "        x_1*x_17 + x_3*x_17 + x_5*x_17 + x_6*x_17 + x_7*x_17 + x_8*x_17 + x_9*x_17 + x_11*x_17 + x_13*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_6*x_18 + \n"
                       "        x_7*x_18 + x_8*x_18 + x_9*x_18 + x_11*x_18 + x_17*x_18 + x_1*x_19 + x_2*x_19 + x_3*x_19 + x_4*x_19 + x_8*x_19 + x_13*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_18*x_19 + x_1*x_20 + x_2*x_20 + \n"
                       "        x_5*x_20 + x_7*x_20 + x_10*x_20 + x_12*x_20 + x_14*x_20 + x_18*x_20 + x_1*x_21 + x_2*x_21 + x_4*x_21 + x_6*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_12*x_21 + x_15*x_21 + x_17*x_21 + x_20*x_21 + \n"
                       "        x_1*x_22 + x_3*x_22 + x_5*x_22 + x_6*x_22 + x_7*x_22 + x_9*x_22 + x_14*x_22 + x_15*x_22 + x_16*x_22 + x_17*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_3*x_23 + x_5*x_23 + x_6*x_23 + \n"
                       "        x_8*x_23 + x_9*x_23 + x_13*x_23 + x_14*x_23 + x_15*x_23 + x_16*x_23 + x_17*x_23 + x_22*x_23 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_12*x_24 + x_16*x_24 + x_20*x_24 +\n"
                       "        x_22*x_24 + x_3*x_25 + x_5*x_25 + x_6*x_25 + x_9*x_25 + x_11*x_25 + x_12*x_25 + x_16*x_25 + x_17*x_25 + x_19*x_25 + x_21*x_25 + x_23*x_25 + x_1*x_26 + x_2*x_26 + x_4*x_26 + x_5*x_26 + x_6*x_26 + \n"
                       "        x_7*x_26 + x_10*x_26 + x_12*x_26 + x_14*x_26 + x_15*x_26 + x_23*x_26 + x_2*x_27 + x_6*x_27 + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_10*x_27 + x_12*x_27 + x_13*x_27 + x_14*x_27 + x_16*x_27 + x_18*x_27 \n"
                       "        + x_21*x_27 + x_22*x_27 + x_25*x_27 + x_26*x_27 + x_1*x_28 + x_2*x_28 + x_4*x_28 + x_6*x_28 + x_8*x_28 + x_10*x_28 + x_11*x_28 + x_12*x_28 + x_15*x_28 + x_17*x_28 + x_18*x_28 + x_21*x_28 + \n"
                       "        x_22*x_28 + x_24*x_28 + x_26*x_28 + x_27*x_28 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_6*x_29 + x_9*x_29 + x_10*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 \n"
                       "        + x_23*x_29 + x_24*x_29 + x_25*x_29 + x_26*x_29 + x_28*x_29 + x_1*x_30 + x_6*x_30 + x_11*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_17*x_30 + x_19*x_30 + x_20*x_30 + x_22*x_30 + x_23*x_30 + \n"
                       "        x_25*x_30 + x_26*x_30 + x_29*x_30 + x_2*x_31 + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_12*x_31 + x_18*x_31 + x_19*x_31 + x_20*x_31 + x_24*x_31 + x_27*x_31 + x_4*x_32 + x_5*x_32 + x_6*x_32 +\n"
                       "        x_9*x_32 + x_13*x_32 + x_15*x_32 + x_16*x_32 + x_19*x_32 + x_28*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_3*x_33 + x_8*x_33 + x_9*x_33 + x_12*x_33 + x_14*x_33 + x_15*x_33 + x_18*x_33 + x_19*x_33\n"
                       "        + x_21*x_33 + x_24*x_33 + x_29*x_33 + x_30*x_33 + x_31*x_33 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_10*x_34 + x_11*x_34 + x_13*x_34 + x_18*x_34 + x_22*x_34 + x_23*x_34 + x_24*x_34 + x_26*x_34 + \n"
                       "        x_28*x_34 + x_29*x_34 + x_30*x_34 + x_2*x_35 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_7*x_35 + x_8*x_35 + x_9*x_35 + x_11*x_35 + x_12*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_22*x_35 + x_23*x_35 +\n"
                       "        x_26*x_35 + x_28*x_35 + x_30*x_35 + x_31*x_35 + x_33*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_8*x_36 + x_9*x_36 + x_11*x_36 + x_13*x_36 + x_15*x_36 + x_16*x_36 + x_24*x_36 +\n"
                       "        x_25*x_36 + x_27*x_36 + x_29*x_36 + x_31*x_36 + x_33*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_7*x_37 + x_8*x_37 + x_11*x_37 + x_12*x_37 + x_13*x_37 + x_14*x_37 \n"
                       "        + x_16*x_37 + x_17*x_37 + x_23*x_37 + x_28*x_37 + x_29*x_37 + x_32*x_37 + x_34*x_37 + x_36*x_37 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_5*x_38 + x_6*x_38 + x_7*x_38 + x_15*x_38 + x_16*x_38 + \n"
                       "        x_18*x_38 + x_21*x_38 + x_24*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_33*x_38 + x_34*x_38 + x_1*x_39 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_7*x_39 + x_8*x_39 + x_9*x_39 + x_10*x_39 + x_11*x_39 +\n"
                       "        x_12*x_39 + x_13*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_17*x_39 + x_18*x_39 + x_20*x_39 + x_22*x_39 + x_26*x_39 + x_28*x_39 + x_30*x_39 + x_33*x_39 + x_37*x_39 + x_38*x_39 + x_4*x_40 + \n"
                       "        x_6*x_40 + x_8*x_40 + x_13*x_40 + x_14*x_40 + x_16*x_40 + x_17*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_26*x_40 + x_30*x_40 + x_32*x_40 + x_34*x_40 + x_35*x_40 + \n"
                       "        x_37*x_40 + x_38*x_40 + x_39*x_40 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_19*x_41 + x_22*x_41 + x_24*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + \n"
                       "        x_29*x_41 + x_34*x_41 + x_36*x_41 + x_37*x_41 + x_1*x_42 + x_2*x_42 + x_6*x_42 + x_8*x_42 + x_9*x_42 + x_14*x_42 + x_15*x_42 + x_17*x_42 + x_18*x_42 + x_20*x_42 + x_21*x_42 + x_23*x_42 + x_26*x_42\n"
                       "        + x_28*x_42 + x_30*x_42 + x_31*x_42 + x_33*x_42 + x_34*x_42 + x_36*x_42 + x_37*x_42 + x_2*x_43 + x_3*x_43 + x_4*x_43 + x_8*x_43 + x_9*x_43 + x_10*x_43 + x_11*x_43 + x_13*x_43 + x_14*x_43 + \n"
                       "        x_17*x_43 + x_18*x_43 + x_19*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + x_30*x_43 + x_32*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_41*x_43 + x_42*x_43 + x_1*x_44 + x_3*x_44 + \n"
                       "        x_5*x_44 + x_7*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_19*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_25*x_44 + x_27*x_44 + x_28*x_44 + x_31*x_44 + x_33*x_44 + x_34*x_44 + \n"
                       "        x_37*x_44 + x_39*x_44 + x_40*x_44 + x_42*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_11*x_45 + x_13*x_45 + x_14*x_45 + x_16*x_45 + x_17*x_45 + x_20*x_45 + x_21*x_45\n"
                       "        + x_28*x_45 + x_31*x_45 + x_32*x_45 + x_39*x_45 + x_1*x_46 + x_4*x_46 + x_5*x_46 + x_6*x_46 + x_7*x_46 + x_9*x_46 + x_11*x_46 + x_16*x_46 + x_17*x_46 + x_20*x_46 + x_21*x_46 + x_23*x_46 + \n"
                       "        x_27*x_46 + x_30*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_40*x_46 + x_43*x_46 + x_45*x_46 + x_1*x_47 + x_2*x_47 + x_3*x_47 + x_4*x_47 + x_7*x_47 + x_8*x_47 + x_9*x_47 + x_11*x_47 + x_13*x_47 +\n"
                       "        x_15*x_47 + x_16*x_47 + x_17*x_47 + x_18*x_47 + x_20*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_24*x_47 + x_25*x_47 + x_27*x_47 + x_30*x_47 + x_35*x_47 + x_37*x_47 + x_40*x_47 + x_41*x_47 + \n"
                       "        x_42*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_46*x_47 + x_1*x_48 + x_6*x_48 + x_8*x_48 + x_12*x_48 + x_13*x_48 + x_14*x_48 + x_16*x_48 + x_17*x_48 + x_18*x_48 + x_20*x_48 + x_22*x_48 + \n"
                       "        x_23*x_48 + x_26*x_48 + x_30*x_48 + x_33*x_48 + x_34*x_48 + x_36*x_48 + x_37*x_48 + x_41*x_48 + x_47*x_48 + x_5*x_49 + x_6*x_49 + x_9*x_49 + x_11*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + \n"
                       "        x_20*x_49 + x_21*x_49 + x_23*x_49 + x_29*x_49 + x_30*x_49 + x_31*x_49 + x_33*x_49 + x_35*x_49 + x_41*x_49 + x_42*x_49 + x_43*x_49 + x_46*x_49 + x_2 + x_3 + x_4 + x_5 + x_7 + x_11 + x_14 + x_16 + \n"
                       "        x_17 + x_18 + x_19 + x_20 + x_23 + x_28 + x_29 + x_30 + x_31 + x_33 + x_35 + x_38 + x_39 + x_41 + x_43 + x_49,\n"
                       "    x_1*x_3 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_5 + x_4*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + x_5*x_6 + x_1*x_7 + x_5*x_7 + x_6*x_7 + x_2*x_8 + x_6*x_8 + x_1*x_9 + x_4*x_9 + x_1*x_10 + x_2*x_10 + x_7*x_10\n"
                       "        + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_7*x_11 + x_9*x_11 + x_10*x_11 + x_1*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_11*x_12 + x_1*x_13 + x_3*x_13 + x_7*x_13 + x_9*x_13 + x_10*x_13 + \n"
                       "        x_11*x_13 + x_12*x_13 + x_1*x_14 + x_4*x_14 + x_5*x_14 + x_6*x_14 + x_10*x_14 + x_12*x_14 + x_13*x_14 + x_5*x_15 + x_6*x_15 + x_10*x_15 + x_12*x_15 + x_13*x_15 + x_2*x_16 + x_3*x_16 + x_6*x_16 + \n"
                       "        x_13*x_16 + x_14*x_16 + x_15*x_16 + x_2*x_17 + x_3*x_17 + x_5*x_17 + x_6*x_17 + x_7*x_17 + x_14*x_17 + x_15*x_17 + x_3*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_8*x_18 + x_9*x_18 + x_10*x_18 + \n"
                       "        x_11*x_18 + x_12*x_18 + x_14*x_18 + x_15*x_18 + x_16*x_18 + x_17*x_18 + x_2*x_19 + x_11*x_19 + x_13*x_19 + x_14*x_19 + x_18*x_19 + x_2*x_20 + x_4*x_20 + x_9*x_20 + x_10*x_20 + x_11*x_20 + \n"
                       "        x_12*x_20 + x_16*x_20 + x_19*x_20 + x_1*x_21 + x_3*x_21 + x_13*x_21 + x_15*x_21 + x_16*x_21 + x_18*x_21 + x_20*x_21 + x_2*x_22 + x_8*x_22 + x_9*x_22 + x_10*x_22 + x_15*x_22 + x_17*x_22 + x_19*x_22\n"
                       "        + x_20*x_22 + x_2*x_23 + x_6*x_23 + x_7*x_23 + x_8*x_23 + x_12*x_23 + x_14*x_23 + x_15*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_20*x_23 + x_21*x_23 + x_1*x_24 + x_2*x_24 + x_4*x_24 + x_6*x_24 \n"
                       "        + x_7*x_24 + x_9*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_17*x_24 + x_19*x_24 + x_21*x_24 + x_1*x_25 + x_2*x_25 + x_3*x_25 + x_5*x_25 + x_6*x_25 + x_8*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 \n"
                       "        + x_15*x_25 + x_16*x_25 + x_20*x_25 + x_23*x_25 + x_24*x_25 + x_2*x_26 + x_6*x_26 + x_7*x_26 + x_10*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_20*x_26 + x_22*x_26 + x_24*x_26 + x_2*x_27 + \n"
                       "        x_4*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_8*x_27 + x_10*x_27 + x_12*x_27 + x_14*x_27 + x_17*x_27 + x_19*x_27 + x_21*x_27 + x_22*x_27 + x_1*x_28 + x_4*x_28 + x_5*x_28 + x_7*x_28 + x_8*x_28 + \n"
                       "        x_9*x_28 + x_13*x_28 + x_19*x_28 + x_20*x_28 + x_25*x_28 + x_26*x_28 + x_1*x_29 + x_2*x_29 + x_4*x_29 + x_6*x_29 + x_11*x_29 + x_12*x_29 + x_14*x_29 + x_18*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29\n"
                       "        + x_24*x_29 + x_26*x_29 + x_27*x_29 + x_1*x_30 + x_2*x_30 + x_3*x_30 + x_5*x_30 + x_6*x_30 + x_8*x_30 + x_10*x_30 + x_12*x_30 + x_15*x_30 + x_16*x_30 + x_18*x_30 + x_19*x_30 + x_23*x_30 + \n"
                       "        x_24*x_30 + x_25*x_30 + x_26*x_30 + x_1*x_31 + x_2*x_31 + x_3*x_31 + x_4*x_31 + x_6*x_31 + x_10*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_21*x_31 + x_22*x_31\n"
                       "        + x_25*x_31 + x_26*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32 + x_8*x_32 + x_10*x_32 + x_12*x_32 + x_14*x_32 + x_16*x_32 + x_17*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + \n"
                       "        x_26*x_32 + x_27*x_32 + x_29*x_32 + x_1*x_33 + x_4*x_33 + x_5*x_33 + x_14*x_33 + x_15*x_33 + x_16*x_33 + x_20*x_33 + x_23*x_33 + x_24*x_33 + x_25*x_33 + x_27*x_33 + x_29*x_33 + x_32*x_33 + \n"
                       "        x_1*x_34 + x_2*x_34 + x_7*x_34 + x_8*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_16*x_34 + x_18*x_34 + x_20*x_34 + x_22*x_34 + x_25*x_34 + x_26*x_34 + x_29*x_34 + x_30*x_34 + \n"
                       "        x_31*x_34 + x_32*x_34 + x_1*x_35 + x_5*x_35 + x_7*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_14*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_20*x_35 + x_22*x_35 + x_23*x_35 + x_26*x_35\n"
                       "        + x_32*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_4*x_36 + x_6*x_36 + x_8*x_36 + x_9*x_36 + x_12*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + x_18*x_36 + x_19*x_36 + x_20*x_36 + \n"
                       "        x_22*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_29*x_36 + x_35*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_6*x_37 + x_9*x_37 + x_10*x_37 + x_12*x_37 + x_13*x_37 + x_14*x_37 + x_19*x_37\n"
                       "        + x_21*x_37 + x_24*x_37 + x_25*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + x_34*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + x_3*x_38 + x_4*x_38 + x_6*x_38 + x_7*x_38 + \n"
                       "        x_8*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_13*x_38 + x_14*x_38 + x_17*x_38 + x_18*x_38 + x_19*x_38 + x_21*x_38 + x_24*x_38 + x_25*x_38 + x_31*x_38 + x_32*x_38 + x_34*x_38 + x_37*x_38 + \n"
                       "        x_3*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_10*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_18*x_39 + x_20*x_39 + x_21*x_39 + x_23*x_39 + x_24*x_39 + x_26*x_39 + x_29*x_39 + x_30*x_39 + \n"
                       "        x_37*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_4*x_40 + x_7*x_40 + x_9*x_40 + x_10*x_40 + x_11*x_40 + x_13*x_40 + x_14*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_19*x_40 + x_20*x_40 \n"
                       "        + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_27*x_40 + x_28*x_40 + x_29*x_40 + x_31*x_40 + x_34*x_40 + x_36*x_40 + x_37*x_40 + x_38*x_40 + x_1*x_41 + x_5*x_41 + x_9*x_41 + x_10*x_41 + x_12*x_41 + \n"
                       "        x_14*x_41 + x_15*x_41 + x_16*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_25*x_41 + x_27*x_41 + x_31*x_41 + x_37*x_41 + x_6*x_42 + x_10*x_42 + x_12*x_42 + x_13*x_42 + x_14*x_42 + x_16*x_42 + \n"
                       "        x_17*x_42 + x_20*x_42 + x_21*x_42 + x_23*x_42 + x_28*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_38*x_42 + x_39*x_42 + x_40*x_42 + x_41*x_42 + x_1*x_43 + x_2*x_43 + x_3*x_43 + x_4*x_43 + \n"
                       "        x_12*x_43 + x_13*x_43 + x_14*x_43 + x_16*x_43 + x_17*x_43 + x_19*x_43 + x_20*x_43 + x_21*x_43 + x_23*x_43 + x_24*x_43 + x_25*x_43 + x_27*x_43 + x_29*x_43 + x_30*x_43 + x_32*x_43 + x_37*x_43 + \n"
                       "        x_38*x_43 + x_40*x_43 + x_41*x_43 + x_42*x_43 + x_3*x_44 + x_4*x_44 + x_5*x_44 + x_6*x_44 + x_12*x_44 + x_13*x_44 + x_15*x_44 + x_16*x_44 + x_19*x_44 + x_20*x_44 + x_21*x_44 + x_25*x_44 + \n"
                       "        x_29*x_44 + x_32*x_44 + x_34*x_44 + x_35*x_44 + x_37*x_44 + x_38*x_44 + x_39*x_44 + x_40*x_44 + x_41*x_44 + x_42*x_44 + x_1*x_45 + x_2*x_45 + x_3*x_45 + x_4*x_45 + x_5*x_45 + x_6*x_45 + x_9*x_45 +\n"
                       "        x_11*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_18*x_45 + x_20*x_45 + x_22*x_45 + x_25*x_45 + x_27*x_45 + x_33*x_45 + x_34*x_45 + x_36*x_45 + x_38*x_45 + x_39*x_45 + x_40*x_45 + x_43*x_45 + \n"
                       "        x_2*x_46 + x_4*x_46 + x_5*x_46 + x_6*x_46 + x_9*x_46 + x_11*x_46 + x_12*x_46 + x_14*x_46 + x_15*x_46 + x_16*x_46 + x_18*x_46 + x_19*x_46 + x_21*x_46 + x_22*x_46 + x_24*x_46 + x_29*x_46 + x_30*x_46\n"
                       "        + x_31*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_39*x_46 + x_41*x_46 + x_43*x_46 + x_44*x_46 + x_2*x_47 + x_3*x_47 + x_4*x_47 + x_6*x_47 + x_7*x_47 + x_10*x_47 + x_12*x_47 + \n"
                       "        x_15*x_47 + x_18*x_47 + x_19*x_47 + x_22*x_47 + x_23*x_47 + x_24*x_47 + x_26*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_31*x_47 + x_34*x_47 + x_35*x_47 + x_36*x_47 + x_37*x_47 + x_38*x_47 + \n"
                       "        x_40*x_47 + x_41*x_47 + x_42*x_47 + x_44*x_47 + x_3*x_48 + x_4*x_48 + x_5*x_48 + x_7*x_48 + x_11*x_48 + x_12*x_48 + x_14*x_48 + x_16*x_48 + x_17*x_48 + x_18*x_48 + x_21*x_48 + x_22*x_48 + \n"
                       "        x_23*x_48 + x_24*x_48 + x_25*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_33*x_48 + x_35*x_48 + x_38*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_44*x_48 + x_46*x_48 + x_8*x_49 + x_9*x_49 + \n"
                       "        x_11*x_49 + x_13*x_49 + x_15*x_49 + x_23*x_49 + x_27*x_49 + x_28*x_49 + x_30*x_49 + x_33*x_49 + x_35*x_49 + x_40*x_49 + x_44*x_49 + x_48*x_49 + x_2 + x_3 + x_4 + x_6 + x_10 + x_12 + x_14 + x_16 + \n"
                       "        x_19 + x_21 + x_22 + x_30 + x_33 + x_35 + x_40 + x_41 + x_42 + x_43 + x_45 + x_46 + x_47,\n"
                       "    x_1*x_2 + x_2*x_3 + x_1*x_4 + x_3*x_4 + x_2*x_5 + x_3*x_5 + x_3*x_6 + x_4*x_6 + x_5*x_6 + x_3*x_7 + x_5*x_7 + x_4*x_8 + x_5*x_8 + x_6*x_8 + x_6*x_9 + x_8*x_9 + x_2*x_10 + x_3*x_10 + x_5*x_10 + \n"
                       "        x_6*x_10 + x_7*x_10 + x_8*x_10 + x_1*x_11 + x_5*x_11 + x_7*x_11 + x_9*x_11 + x_10*x_11 + x_2*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_7*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_4*x_13 + x_5*x_13\n"
                       "        + x_6*x_13 + x_7*x_13 + x_8*x_13 + x_9*x_13 + x_12*x_13 + x_1*x_14 + x_3*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14 + x_9*x_14 + x_11*x_14 + x_12*x_14 + x_1*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + \n"
                       "        x_7*x_15 + x_8*x_15 + x_9*x_15 + x_13*x_15 + x_14*x_15 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_6*x_16 + x_7*x_16 + x_8*x_16 + x_10*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_2*x_17 + x_3*x_17 + \n"
                       "        x_6*x_17 + x_8*x_17 + x_9*x_17 + x_2*x_18 + x_4*x_18 + x_7*x_18 + x_9*x_18 + x_11*x_18 + x_12*x_18 + x_13*x_18 + x_14*x_18 + x_16*x_18 + x_17*x_18 + x_4*x_19 + x_10*x_19 + x_14*x_19 + x_16*x_19 + \n"
                       "        x_17*x_19 + x_1*x_20 + x_4*x_20 + x_5*x_20 + x_9*x_20 + x_11*x_20 + x_12*x_20 + x_13*x_20 + x_15*x_20 + x_16*x_20 + x_18*x_20 + x_19*x_20 + x_1*x_21 + x_2*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + \n"
                       "        x_8*x_21 + x_9*x_21 + x_10*x_21 + x_11*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + x_15*x_21 + x_17*x_21 + x_18*x_21 + x_1*x_22 + x_3*x_22 + x_5*x_22 + x_6*x_22 + x_12*x_22 + x_13*x_22 + x_16*x_22 \n"
                       "        + x_17*x_22 + x_4*x_23 + x_6*x_23 + x_8*x_23 + x_12*x_23 + x_13*x_23 + x_15*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_20*x_23 + x_22*x_23 + x_1*x_24 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_7*x_24 \n"
                       "        + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_14*x_24 + x_15*x_24 + x_17*x_24 + x_19*x_24 + x_20*x_24 + x_23*x_24 + x_1*x_25 + x_4*x_25 + x_5*x_25 + x_6*x_25 + x_7*x_25 + x_8*x_25\n"
                       "        + x_11*x_25 + x_13*x_25 + x_15*x_25 + x_16*x_25 + x_19*x_25 + x_20*x_25 + x_1*x_26 + x_2*x_26 + x_4*x_26 + x_6*x_26 + x_7*x_26 + x_11*x_26 + x_12*x_26 + x_13*x_26 + x_15*x_26 + x_17*x_26 + \n"
                       "        x_18*x_26 + x_19*x_26 + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_3*x_27 + x_4*x_27 + x_5*x_27 + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_11*x_27 + x_14*x_27 + \n"
                       "        x_16*x_27 + x_17*x_27 + x_19*x_27 + x_20*x_27 + x_24*x_27 + x_2*x_28 + x_3*x_28 + x_7*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + x_15*x_28 + x_17*x_28 + x_19*x_28 + x_20*x_28 + x_21*x_28 + \n"
                       "        x_22*x_28 + x_23*x_28 + x_26*x_28 + x_27*x_28 + x_2*x_29 + x_3*x_29 + x_6*x_29 + x_7*x_29 + x_8*x_29 + x_9*x_29 + x_11*x_29 + x_12*x_29 + x_13*x_29 + x_14*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 \n"
                       "        + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_24*x_29 + x_26*x_29 + x_28*x_29 + x_3*x_30 + x_4*x_30 + x_5*x_30 + x_10*x_30 + x_12*x_30 + x_13*x_30 + x_15*x_30 + x_16*x_30 + x_20*x_30 + x_21*x_30 + \n"
                       "        x_24*x_30 + x_25*x_30 + x_28*x_30 + x_29*x_30 + x_1*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 +\n"
                       "        x_18*x_31 + x_20*x_31 + x_22*x_31 + x_23*x_31 + x_24*x_31 + x_26*x_31 + x_27*x_31 + x_28*x_31 + x_29*x_31 + x_1*x_32 + x_4*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_9*x_32 + x_10*x_32 + x_11*x_32 \n"
                       "        + x_14*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_22*x_32 + x_23*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_1*x_33 + x_2*x_33 + x_8*x_33 + x_11*x_33 + x_14*x_33 + \n"
                       "        x_15*x_33 + x_17*x_33 + x_18*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_23*x_33 + x_25*x_33 + x_27*x_33 + x_30*x_33 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_8*x_34 + x_9*x_34 +\n"
                       "        x_12*x_34 + x_13*x_34 + x_15*x_34 + x_17*x_34 + x_24*x_34 + x_28*x_34 + x_30*x_34 + x_31*x_34 + x_2*x_35 + x_4*x_35 + x_6*x_35 + x_8*x_35 + x_9*x_35 + x_11*x_35 + x_13*x_35 + x_15*x_35 + x_17*x_35\n"
                       "        + x_21*x_35 + x_22*x_35 + x_24*x_35 + x_25*x_35 + x_26*x_35 + x_28*x_35 + x_31*x_35 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_8*x_36 + x_9*x_36 + x_11*x_36 + x_13*x_36 + x_14*x_36 + \n"
                       "        x_16*x_36 + x_18*x_36 + x_19*x_36 + x_20*x_36 + x_21*x_36 + x_24*x_36 + x_26*x_36 + x_27*x_36 + x_29*x_36 + x_30*x_36 + x_32*x_36 + x_33*x_36 + x_4*x_37 + x_5*x_37 + x_9*x_37 + x_10*x_37 + \n"
                       "        x_14*x_37 + x_16*x_37 + x_17*x_37 + x_18*x_37 + x_19*x_37 + x_22*x_37 + x_23*x_37 + x_25*x_37 + x_29*x_37 + x_30*x_37 + x_32*x_37 + x_34*x_37 + x_35*x_37 + x_1*x_38 + x_4*x_38 + x_12*x_38 + \n"
                       "        x_13*x_38 + x_15*x_38 + x_17*x_38 + x_18*x_38 + x_19*x_38 + x_21*x_38 + x_24*x_38 + x_28*x_38 + x_30*x_38 + x_36*x_38 + x_1*x_39 + x_2*x_39 + x_3*x_39 + x_4*x_39 + x_6*x_39 + x_9*x_39 + x_10*x_39 \n"
                       "        + x_11*x_39 + x_14*x_39 + x_16*x_39 + x_17*x_39 + x_23*x_39 + x_26*x_39 + x_28*x_39 + x_31*x_39 + x_33*x_39 + x_37*x_39 + x_2*x_40 + x_3*x_40 + x_6*x_40 + x_8*x_40 + x_9*x_40 + x_13*x_40 + \n"
                       "        x_14*x_40 + x_16*x_40 + x_17*x_40 + x_19*x_40 + x_20*x_40 + x_22*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_30*x_40 + x_32*x_40 + x_33*x_40 + x_34*x_40 + x_37*x_40 + x_1*x_41 + x_2*x_41 + \n"
                       "        x_8*x_41 + x_10*x_41 + x_11*x_41 + x_12*x_41 + x_14*x_41 + x_15*x_41 + x_18*x_41 + x_19*x_41 + x_21*x_41 + x_23*x_41 + x_25*x_41 + x_27*x_41 + x_28*x_41 + x_31*x_41 + x_34*x_41 + x_35*x_41 + \n"
                       "        x_38*x_41 + x_40*x_41 + x_1*x_42 + x_2*x_42 + x_3*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_11*x_42 + x_14*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_24*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 \n"
                       "        + x_32*x_42 + x_34*x_42 + x_36*x_42 + x_38*x_42 + x_39*x_42 + x_41*x_42 + x_1*x_43 + x_3*x_43 + x_4*x_43 + x_7*x_43 + x_11*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_16*x_43 + x_17*x_43 + \n"
                       "        x_19*x_43 + x_22*x_43 + x_23*x_43 + x_26*x_43 + x_27*x_43 + x_30*x_43 + x_33*x_43 + x_34*x_43 + x_37*x_43 + x_38*x_43 + x_40*x_43 + x_5*x_44 + x_7*x_44 + x_8*x_44 + x_11*x_44 + x_13*x_44 + \n"
                       "        x_20*x_44 + x_22*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_31*x_44 + x_36*x_44 + x_37*x_44 + x_2*x_45 + x_7*x_45 + x_9*x_45 + x_10*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_16*x_45 + \n"
                       "        x_18*x_45 + x_20*x_45 + x_23*x_45 + x_24*x_45 + x_26*x_45 + x_31*x_45 + x_32*x_45 + x_34*x_45 + x_36*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + x_43*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + \n"
                       "        x_5*x_46 + x_6*x_46 + x_8*x_46 + x_10*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + x_15*x_46 + x_17*x_46 + x_18*x_46 + x_20*x_46 + x_22*x_46 + x_24*x_46 + x_26*x_46 + x_29*x_46 + x_32*x_46 + \n"
                       "        x_34*x_46 + x_35*x_46 + x_36*x_46 + x_38*x_46 + x_40*x_46 + x_44*x_46 + x_1*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_15*x_47 + x_16*x_47 + x_17*x_47 + x_19*x_47 + x_21*x_47 + \n"
                       "        x_23*x_47 + x_24*x_47 + x_26*x_47 + x_29*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_36*x_47 + x_37*x_47 + x_39*x_47 + x_44*x_47 + x_46*x_47 + x_3*x_48 + \n"
                       "        x_4*x_48 + x_5*x_48 + x_9*x_48 + x_13*x_48 + x_15*x_48 + x_16*x_48 + x_18*x_48 + x_19*x_48 + x_23*x_48 + x_24*x_48 + x_26*x_48 + x_27*x_48 + x_28*x_48 + x_30*x_48 + x_33*x_48 + x_35*x_48 + \n"
                       "        x_39*x_48 + x_40*x_48 + x_41*x_48 + x_45*x_48 + x_1*x_49 + x_3*x_49 + x_4*x_49 + x_5*x_49 + x_6*x_49 + x_9*x_49 + x_11*x_49 + x_13*x_49 + x_17*x_49 + x_18*x_49 + x_19*x_49 + x_22*x_49 + x_23*x_49 \n"
                       "        + x_26*x_49 + x_29*x_49 + x_30*x_49 + x_32*x_49 + x_36*x_49 + x_37*x_49 + x_40*x_49 + x_45*x_49 + x_48*x_49 + x_6 + x_7 + x_9 + x_10 + x_11 + x_15 + x_18 + x_19 + x_20 + x_22 + x_24 + x_28 + x_30 \n"
                       "        + x_31 + x_34 + x_35 + x_36 + x_37 + x_39 + x_41 + x_42 + x_43 + x_46 + x_49,\n"
                       "    x_1*x_3 + x_2*x_3 + x_1*x_4 + x_2*x_4 + x_1*x_5 + x_1*x_6 + x_4*x_6 + x_2*x_7 + x_4*x_7 + x_6*x_7 + x_2*x_8 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_5*x_9 + x_7*x_9 + x_3*x_10 +\n"
                       "        x_4*x_10 + x_9*x_10 + x_1*x_11 + x_2*x_11 + x_3*x_11 + x_6*x_11 + x_7*x_11 + x_8*x_11 + x_9*x_11 + x_1*x_12 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_9*x_12 + x_10*x_12 + x_1*x_13 + x_3*x_13\n"
                       "        + x_4*x_13 + x_6*x_13 + x_7*x_13 + x_9*x_13 + x_10*x_13 + x_11*x_13 + x_12*x_13 + x_5*x_14 + x_7*x_14 + x_8*x_14 + x_9*x_14 + x_2*x_15 + x_4*x_15 + x_6*x_15 + x_7*x_15 + x_11*x_15 + x_12*x_15 + \n"
                       "        x_13*x_15 + x_1*x_16 + x_3*x_16 + x_4*x_16 + x_8*x_16 + x_11*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_2*x_17 + x_5*x_17 + x_6*x_17 + x_8*x_17 + x_9*x_17 + x_11*x_17 + x_13*x_17 + x_14*x_17 + \n"
                       "        x_1*x_18 + x_3*x_18 + x_5*x_18 + x_8*x_18 + x_9*x_18 + x_11*x_18 + x_15*x_18 + x_16*x_18 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_6*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_15*x_19 + \n"
                       "        x_17*x_19 + x_1*x_20 + x_3*x_20 + x_5*x_20 + x_8*x_20 + x_9*x_20 + x_10*x_20 + x_11*x_20 + x_12*x_20 + x_14*x_20 + x_15*x_20 + x_17*x_20 + x_4*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + \n"
                       "        x_12*x_21 + x_13*x_21 + x_14*x_21 + x_17*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_4*x_22 + x_6*x_22 + x_7*x_22 + x_8*x_22 + x_9*x_22 + x_11*x_22 + x_13*x_22 + \n"
                       "        x_14*x_22 + x_15*x_22 + x_18*x_22 + x_19*x_22 + x_20*x_22 + x_1*x_23 + x_2*x_23 + x_4*x_23 + x_5*x_23 + x_7*x_23 + x_8*x_23 + x_13*x_23 + x_14*x_23 + x_17*x_23 + x_21*x_23 + x_1*x_24 + x_4*x_24 + \n"
                       "        x_10*x_24 + x_15*x_24 + x_16*x_24 + x_17*x_24 + x_1*x_25 + x_3*x_25 + x_5*x_25 + x_8*x_25 + x_9*x_25 + x_11*x_25 + x_12*x_25 + x_14*x_25 + x_15*x_25 + x_20*x_25 + x_23*x_25 + x_1*x_26 + x_6*x_26 +\n"
                       "        x_9*x_26 + x_10*x_26 + x_12*x_26 + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_23*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_8*x_27 + x_10*x_27 + x_11*x_27 +\n"
                       "        x_12*x_27 + x_13*x_27 + x_14*x_27 + x_17*x_27 + x_18*x_27 + x_19*x_27 + x_20*x_27 + x_21*x_27 + x_22*x_27 + x_23*x_27 + x_25*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_9*x_28 + x_10*x_28\n"
                       "        + x_13*x_28 + x_17*x_28 + x_18*x_28 + x_19*x_28 + x_21*x_28 + x_24*x_28 + x_25*x_28 + x_27*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_7*x_29 + x_9*x_29 + x_11*x_29 + x_12*x_29\n"
                       "        + x_16*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_23*x_29 + x_26*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 + x_3*x_30 + x_4*x_30 + x_5*x_30 + x_6*x_30 + x_7*x_30\n"
                       "        + x_9*x_30 + x_15*x_30 + x_16*x_30 + x_18*x_30 + x_20*x_30 + x_25*x_30 + x_27*x_30 + x_29*x_30 + x_2*x_31 + x_6*x_31 + x_7*x_31 + x_8*x_31 + x_10*x_31 + x_11*x_31 + x_12*x_31 + x_15*x_31 + \n"
                       "        x_16*x_31 + x_22*x_31 + x_23*x_31 + x_24*x_31 + x_26*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_1*x_32 + x_6*x_32 + x_13*x_32 + x_17*x_32 + x_19*x_32 + x_22*x_32 + x_24*x_32 + x_27*x_32 + \n"
                       "        x_29*x_32 + x_30*x_32 + x_2*x_33 + x_4*x_33 + x_5*x_33 + x_9*x_33 + x_10*x_33 + x_13*x_33 + x_14*x_33 + x_15*x_33 + x_18*x_33 + x_20*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_29*x_33 + \n"
                       "        x_30*x_33 + x_31*x_33 + x_7*x_34 + x_8*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_19*x_34 + x_21*x_34 + x_22*x_34 + x_24*x_34 + x_25*x_34 + x_27*x_34 + x_28*x_34 + x_29*x_34 + x_30*x_34 + \n"
                       "        x_32*x_34 + x_2*x_35 + x_8*x_35 + x_11*x_35 + x_20*x_35 + x_21*x_35 + x_22*x_35 + x_23*x_35 + x_27*x_35 + x_29*x_35 + x_30*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_3*x_36 + x_4*x_36 + x_6*x_36 \n"
                       "        + x_7*x_36 + x_12*x_36 + x_16*x_36 + x_18*x_36 + x_21*x_36 + x_25*x_36 + x_29*x_36 + x_30*x_36 + x_32*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_12*x_37 + x_14*x_37 + x_16*x_37 + \n"
                       "        x_24*x_37 + x_27*x_37 + x_29*x_37 + x_30*x_37 + x_34*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_5*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_13*x_38 + \n"
                       "        x_14*x_38 + x_16*x_38 + x_17*x_38 + x_20*x_38 + x_22*x_38 + x_23*x_38 + x_26*x_38 + x_27*x_38 + x_30*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_34*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + \n"
                       "        x_5*x_39 + x_6*x_39 + x_7*x_39 + x_11*x_39 + x_13*x_39 + x_14*x_39 + x_15*x_39 + x_17*x_39 + x_19*x_39 + x_22*x_39 + x_24*x_39 + x_25*x_39 + x_27*x_39 + x_33*x_39 + x_35*x_39 + x_36*x_39 + \n"
                       "        x_37*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_6*x_40 + x_9*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_21*x_40 + x_22*x_40 + x_23*x_40 + x_24*x_40\n"
                       "        + x_26*x_40 + x_27*x_40 + x_29*x_40 + x_31*x_40 + x_35*x_40 + x_37*x_40 + x_1*x_41 + x_4*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_11*x_41 + x_14*x_41 + x_16*x_41 + x_18*x_41 + \n"
                       "        x_22*x_41 + x_23*x_41 + x_24*x_41 + x_26*x_41 + x_27*x_41 + x_28*x_41 + x_29*x_41 + x_32*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_36*x_41 + x_37*x_41 + x_38*x_41 + x_1*x_42 + x_4*x_42 + \n"
                       "        x_8*x_42 + x_13*x_42 + x_15*x_42 + x_16*x_42 + x_17*x_42 + x_20*x_42 + x_21*x_42 + x_22*x_42 + x_23*x_42 + x_24*x_42 + x_26*x_42 + x_28*x_42 + x_30*x_42 + x_31*x_42 + x_33*x_42 + x_34*x_42 + \n"
                       "        x_35*x_42 + x_36*x_42 + x_38*x_42 + x_39*x_42 + x_40*x_42 + x_41*x_42 + x_1*x_43 + x_2*x_43 + x_9*x_43 + x_10*x_43 + x_11*x_43 + x_13*x_43 + x_14*x_43 + x_17*x_43 + x_18*x_43 + x_19*x_43 + \n"
                       "        x_20*x_43 + x_21*x_43 + x_23*x_43 + x_25*x_43 + x_26*x_43 + x_27*x_43 + x_28*x_43 + x_29*x_43 + x_30*x_43 + x_32*x_43 + x_33*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + \n"
                       "        x_41*x_43 + x_42*x_43 + x_1*x_44 + x_2*x_44 + x_3*x_44 + x_5*x_44 + x_8*x_44 + x_9*x_44 + x_12*x_44 + x_13*x_44 + x_16*x_44 + x_17*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_27*x_44 \n"
                       "        + x_28*x_44 + x_36*x_44 + x_40*x_44 + x_42*x_44 + x_3*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_9*x_45 + x_10*x_45 + x_11*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_18*x_45 + \n"
                       "        x_20*x_45 + x_28*x_45 + x_29*x_45 + x_31*x_45 + x_33*x_45 + x_34*x_45 + x_35*x_45 + x_37*x_45 + x_40*x_45 + x_42*x_45 + x_43*x_45 + x_2*x_46 + x_4*x_46 + x_6*x_46 + x_8*x_46 + x_9*x_46 + x_10*x_46\n"
                       "        + x_12*x_46 + x_16*x_46 + x_17*x_46 + x_18*x_46 + x_19*x_46 + x_20*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_28*x_46 + x_40*x_46 + x_2*x_47 + x_4*x_47 + x_6*x_47 + x_7*x_47 + x_8*x_47 + \n"
                       "        x_9*x_47 + x_10*x_47 + x_11*x_47 + x_14*x_47 + x_19*x_47 + x_21*x_47 + x_22*x_47 + x_27*x_47 + x_29*x_47 + x_30*x_47 + x_32*x_47 + x_38*x_47 + x_39*x_47 + x_41*x_47 + x_43*x_47 + x_45*x_47 + \n"
                       "        x_2*x_48 + x_4*x_48 + x_6*x_48 + x_7*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_15*x_48 + x_24*x_48 + x_25*x_48 + x_26*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_34*x_48 + x_38*x_48 + \n"
                       "        x_39*x_48 + x_42*x_48 + x_45*x_48 + x_1*x_49 + x_3*x_49 + x_4*x_49 + x_6*x_49 + x_7*x_49 + x_11*x_49 + x_12*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49 + x_22*x_49 + x_23*x_49 + x_25*x_49 + x_30*x_49\n"
                       "        + x_31*x_49 + x_37*x_49 + x_39*x_49 + x_40*x_49 + x_41*x_49 + x_44*x_49 + x_46*x_49 + x_47*x_49 + x_3 + x_4 + x_9 + x_13 + x_14 + x_17 + x_19 + x_24 + x_25 + x_27 + x_30 + x_33 + x_34 + x_37 + \n"
                       "        x_38 + x_39 + x_42 + x_43 + x_44 + x_48 + x_49,\n"
                       "    x_1*x_4 + x_2*x_5 + x_4*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + x_4*x_6 + x_5*x_6 + x_1*x_7 + x_4*x_7 + x_6*x_7 + x_1*x_8 + x_4*x_8 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_3*x_9 + x_5*x_9 + \n"
                       "        x_6*x_9 + x_8*x_9 + x_1*x_10 + x_2*x_10 + x_3*x_10 + x_4*x_10 + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_2*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_8*x_11 + x_9*x_11 + \n"
                       "        x_1*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_10*x_12 + x_1*x_13 + x_4*x_13 + x_6*x_13 + x_9*x_13 + x_10*x_13 + x_1*x_14 + x_2*x_14 + x_6*x_14 + x_8*x_14 + x_10*x_14 + x_13*x_14 + x_1*x_15 + \n"
                       "        x_2*x_15 + x_3*x_15 + x_5*x_15 + x_9*x_15 + x_11*x_15 + x_2*x_16 + x_4*x_16 + x_7*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_3*x_17 + x_4*x_17 + x_6*x_17 + \n"
                       "        x_7*x_17 + x_8*x_17 + x_9*x_17 + x_12*x_17 + x_14*x_17 + x_15*x_17 + x_2*x_18 + x_12*x_18 + x_15*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + x_2*x_19 + x_3*x_19 + x_4*x_19 + x_9*x_19 + x_11*x_19 + \n"
                       "        x_12*x_19 + x_13*x_19 + x_14*x_19 + x_16*x_19 + x_18*x_19 + x_2*x_20 + x_3*x_20 + x_4*x_20 + x_5*x_20 + x_6*x_20 + x_9*x_20 + x_10*x_20 + x_13*x_20 + x_16*x_20 + x_18*x_20 + x_1*x_21 + x_2*x_21 + \n"
                       "        x_3*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_11*x_21 + x_12*x_21 + x_13*x_21 + x_15*x_21 + x_19*x_21 + x_4*x_22 + x_5*x_22 + x_9*x_22 + x_10*x_22 + x_11*x_22 + x_13*x_22 + x_15*x_22 + \n"
                       "        x_17*x_22 + x_18*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_4*x_23 + x_5*x_23 + x_10*x_23 + x_11*x_23 + x_17*x_23 + x_21*x_23 + x_22*x_23 + x_2*x_24 + x_4*x_24 + x_11*x_24 + x_13*x_24 + \n"
                       "        x_16*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_23*x_24 + x_2*x_25 + x_4*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 + x_13*x_25 + x_15*x_25 + x_16*x_25 + x_17*x_25 + x_20*x_25 + x_21*x_25 + \n"
                       "        x_22*x_25 + x_2*x_26 + x_7*x_26 + x_8*x_26 + x_12*x_26 + x_13*x_26 + x_16*x_26 + x_19*x_26 + x_22*x_26 + x_23*x_26 + x_24*x_26 + x_25*x_26 + x_5*x_27 + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_10*x_27 +\n"
                       "        x_11*x_27 + x_13*x_27 + x_15*x_27 + x_17*x_27 + x_19*x_27 + x_22*x_27 + x_3*x_28 + x_5*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_20*x_28 + x_27*x_28 + \n"
                       "        x_5*x_29 + x_8*x_29 + x_10*x_29 + x_11*x_29 + x_13*x_29 + x_14*x_29 + x_17*x_29 + x_19*x_29 + x_26*x_29 + x_27*x_29 + x_1*x_30 + x_3*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_13*x_30 + x_15*x_30 +\n"
                       "        x_16*x_30 + x_20*x_30 + x_21*x_30 + x_22*x_30 + x_24*x_30 + x_25*x_30 + x_26*x_30 + x_28*x_30 + x_29*x_30 + x_1*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 \n"
                       "        + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_19*x_31 + x_20*x_31 + x_22*x_31 + x_24*x_31 + x_30*x_31 + x_2*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_9*x_32 + \n"
                       "        x_10*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_15*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_22*x_32 + x_24*x_32 + x_28*x_32 + x_29*x_32 + x_31*x_32 + \n"
                       "        x_1*x_33 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_5*x_33 + x_11*x_33 + x_13*x_33 + x_16*x_33 + x_19*x_33 + x_21*x_33 + x_23*x_33 + x_26*x_33 + x_32*x_33 + x_1*x_34 + x_2*x_34 + x_4*x_34 + x_6*x_34 + \n"
                       "        x_7*x_34 + x_11*x_34 + x_13*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_22*x_34 + x_27*x_34 + x_1*x_35 + x_3*x_35 + x_4*x_35 + x_7*x_35 + x_10*x_35 + x_13*x_35 + x_17*x_35 + x_21*x_35 + x_22*x_35\n"
                       "        + x_25*x_35 + x_26*x_35 + x_27*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_6*x_36 + x_8*x_36 + x_10*x_36 + x_15*x_36 + x_16*x_36 + x_18*x_36 + x_19*x_36 + \n"
                       "        x_20*x_36 + x_21*x_36 + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_28*x_36 + x_30*x_36 + x_31*x_36 + x_32*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_4*x_37 + \n"
                       "        x_5*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_11*x_37 + x_15*x_37 + x_17*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 + x_24*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37\n"
                       "        + x_34*x_37 + x_36*x_37 + x_1*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + x_11*x_38 + x_12*x_38 + x_16*x_38 + x_18*x_38 + x_20*x_38 + x_27*x_38 + x_31*x_38 + x_35*x_38 + x_2*x_39 + x_4*x_39 + x_6*x_39\n"
                       "        + x_11*x_39 + x_13*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_18*x_39 + x_22*x_39 + x_23*x_39 + x_25*x_39 + x_34*x_39 + x_36*x_39 + x_37*x_39 + x_1*x_40 + x_5*x_40 + x_10*x_40 + x_12*x_40 + \n"
                       "        x_16*x_40 + x_17*x_40 + x_19*x_40 + x_22*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + x_31*x_40 + x_32*x_40 + x_37*x_40 + x_39*x_40 + x_1*x_41 + \n"
                       "        x_2*x_41 + x_3*x_41 + x_5*x_41 + x_6*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_17*x_41 + x_21*x_41 + x_23*x_41 + x_24*x_41 + x_26*x_41 + x_28*x_41 \n"
                       "        + x_32*x_41 + x_34*x_41 + x_36*x_41 + x_38*x_41 + x_40*x_41 + x_1*x_42 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_9*x_42 + x_14*x_42 + x_16*x_42 + x_18*x_42 + x_19*x_42 + x_22*x_42 + x_25*x_42 + \n"
                       "        x_26*x_42 + x_30*x_42 + x_36*x_42 + x_37*x_42 + x_1*x_43 + x_3*x_43 + x_4*x_43 + x_7*x_43 + x_8*x_43 + x_9*x_43 + x_13*x_43 + x_16*x_43 + x_17*x_43 + x_18*x_43 + x_22*x_43 + x_24*x_43 + x_25*x_43 \n"
                       "        + x_28*x_43 + x_30*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + x_1*x_44 + x_2*x_44 + x_3*x_44 + x_6*x_44 + x_8*x_44 + x_9*x_44 + x_10*x_44 + x_12*x_44 + x_15*x_44 + \n"
                       "        x_16*x_44 + x_17*x_44 + x_19*x_44 + x_24*x_44 + x_27*x_44 + x_28*x_44 + x_34*x_44 + x_35*x_44 + x_37*x_44 + x_40*x_44 + x_41*x_44 + x_42*x_44 + x_43*x_44 + x_3*x_45 + x_6*x_45 + x_8*x_45 + \n"
                       "        x_10*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + x_18*x_45 + x_19*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + x_24*x_45 + x_25*x_45 + x_26*x_45 + x_30*x_45 + x_31*x_45 + x_32*x_45 + x_33*x_45 + \n"
                       "        x_34*x_45 + x_35*x_45 + x_39*x_45 + x_41*x_45 + x_42*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_4*x_46 + x_5*x_46 + x_8*x_46 + x_10*x_46 + x_11*x_46 + x_14*x_46 + x_16*x_46 + x_18*x_46 + \n"
                       "        x_19*x_46 + x_21*x_46 + x_24*x_46 + x_27*x_46 + x_30*x_46 + x_35*x_46 + x_36*x_46 + x_38*x_46 + x_40*x_46 + x_4*x_47 + x_5*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + x_13*x_47 + \n"
                       "        x_14*x_47 + x_15*x_47 + x_16*x_47 + x_21*x_47 + x_22*x_47 + x_24*x_47 + x_26*x_47 + x_27*x_47 + x_28*x_47 + x_30*x_47 + x_34*x_47 + x_36*x_47 + x_37*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + \n"
                       "        x_42*x_47 + x_2*x_48 + x_4*x_48 + x_5*x_48 + x_10*x_48 + x_12*x_48 + x_16*x_48 + x_20*x_48 + x_21*x_48 + x_23*x_48 + x_29*x_48 + x_30*x_48 + x_31*x_48 + x_33*x_48 + x_35*x_48 + x_36*x_48 + \n"
                       "        x_37*x_48 + x_40*x_48 + x_43*x_48 + x_45*x_48 + x_46*x_48 + x_1*x_49 + x_5*x_49 + x_6*x_49 + x_7*x_49 + x_8*x_49 + x_11*x_49 + x_12*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_18*x_49\n"
                       "        + x_20*x_49 + x_22*x_49 + x_25*x_49 + x_26*x_49 + x_28*x_49 + x_30*x_49 + x_33*x_49 + x_35*x_49 + x_36*x_49 + x_37*x_49 + x_38*x_49 + x_42*x_49 + x_43*x_49 + x_46*x_49 + x_2 + x_4 + x_6 + x_7 + \n"
                       "        x_8 + x_11 + x_12 + x_13 + x_17 + x_19 + x_20 + x_22 + x_23 + x_24 + x_25 + x_28 + x_30 + x_31 + x_40 + x_41 + x_44 + x_45 + x_46 + x_47 + x_48 + x_49,\n"
                       "    x_1*x_3 + x_2*x_3 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_6 + x_4*x_6 + x_5*x_6 + x_4*x_7 + x_5*x_7 + x_6*x_7 + x_1*x_8 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + \n"
                       "        x_5*x_9 + x_6*x_9 + x_7*x_9 + x_1*x_10 + x_2*x_10 + x_3*x_10 + x_4*x_10 + x_5*x_10 + x_9*x_10 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_7*x_11 + x_8*x_11 + x_1*x_12 + x_2*x_12 + x_3*x_12 + x_4*x_12 + \n"
                       "        x_5*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_3*x_13 + x_6*x_13 + x_7*x_13 + x_10*x_13 + x_2*x_14 + x_4*x_14 + x_6*x_14 + x_7*x_14 + x_8*x_14 + x_10*x_14 + x_11*x_14 + x_13*x_14 + x_1*x_15 + \n"
                       "        x_2*x_15 + x_4*x_15 + x_7*x_15 + x_8*x_15 + x_10*x_15 + x_12*x_15 + x_14*x_15 + x_6*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_14*x_16 + x_1*x_17 + x_2*x_17 + x_3*x_17 + x_6*x_17 + \n"
                       "        x_9*x_17 + x_10*x_17 + x_11*x_17 + x_12*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + x_2*x_18 + x_3*x_18 + x_8*x_18 + x_14*x_18 + x_17*x_18 + x_1*x_19 + x_4*x_19 + x_6*x_19 + x_8*x_19 + \n"
                       "        x_10*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_1*x_20 + x_3*x_20 + x_5*x_20 + x_9*x_20 + x_12*x_20 + x_13*x_20 + x_16*x_20 + x_17*x_20 + x_19*x_20 + x_1*x_21 + x_3*x_21 + x_4*x_21 + x_6*x_21 + \n"
                       "        x_9*x_21 + x_10*x_21 + x_11*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + x_15*x_21 + x_16*x_21 + x_18*x_21 + x_1*x_22 + x_3*x_22 + x_4*x_22 + x_7*x_22 + x_9*x_22 + x_10*x_22 + x_12*x_22 + x_13*x_22 \n"
                       "        + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_3*x_23 + x_5*x_23 + x_9*x_23 + x_10*x_23 + x_11*x_23 + x_14*x_23 + x_15*x_23 + x_17*x_23 + x_20*x_23 + x_21*x_23 + x_22*x_23 + x_10*x_24 + x_11*x_24 + \n"
                       "        x_12*x_24 + x_16*x_24 + x_17*x_24 + x_21*x_24 + x_22*x_24 + x_3*x_25 + x_7*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_13*x_25 + x_14*x_25 + x_15*x_25 + x_16*x_25 + x_17*x_25 + x_18*x_25 + \n"
                       "        x_20*x_25 + x_22*x_25 + x_1*x_26 + x_3*x_26 + x_4*x_26 + x_5*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_11*x_26 + x_12*x_26 + x_18*x_26 + x_19*x_26 + x_21*x_26 + x_23*x_26 + x_24*x_26 + x_25*x_26 +\n"
                       "        x_1*x_27 + x_2*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_8*x_27 + x_15*x_27 + x_19*x_27 + x_21*x_27 + x_22*x_27 + x_24*x_27 + x_25*x_27 + x_26*x_27 + x_2*x_28 + x_3*x_28 + x_5*x_28 + x_6*x_28 + \n"
                       "        x_7*x_28 + x_9*x_28 + x_10*x_28 + x_12*x_28 + x_13*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_21*x_28 + x_22*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_26*x_28 + x_1*x_29 + x_2*x_29 + x_7*x_29\n"
                       "        + x_8*x_29 + x_10*x_29 + x_12*x_29 + x_16*x_29 + x_17*x_29 + x_21*x_29 + x_22*x_29 + x_24*x_29 + x_25*x_29 + x_26*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 + x_4*x_30 + x_6*x_30 + \n"
                       "        x_8*x_30 + x_9*x_30 + x_11*x_30 + x_12*x_30 + x_15*x_30 + x_16*x_30 + x_17*x_30 + x_18*x_30 + x_20*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + x_28*x_30 + x_29*x_30 + x_2*x_31 + x_4*x_31 + x_5*x_31\n"
                       "        + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_11*x_31 + x_17*x_31 + x_18*x_31 + x_22*x_31 + x_24*x_31 + x_25*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_3*x_32 + x_4*x_32 + x_5*x_32 + x_8*x_32 + x_9*x_32 \n"
                       "        + x_11*x_32 + x_13*x_32 + x_14*x_32 + x_17*x_32 + x_19*x_32 + x_21*x_32 + x_22*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_2*x_33 + \n"
                       "        x_9*x_33 + x_10*x_33 + x_12*x_33 + x_14*x_33 + x_16*x_33 + x_20*x_33 + x_22*x_33 + x_23*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_28*x_33 + x_29*x_33 + x_30*x_33 + x_31*x_33 + \n"
                       "        x_32*x_33 + x_1*x_34 + x_7*x_34 + x_8*x_34 + x_10*x_34 + x_16*x_34 + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_22*x_34 + x_23*x_34 + x_25*x_34 + x_27*x_34 + x_28*x_34 + x_30*x_34 + \n"
                       "        x_31*x_34 + x_3*x_35 + x_6*x_35 + x_7*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_12*x_35 + x_13*x_35 + x_15*x_35 + x_17*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35 + x_23*x_35 + x_24*x_35 + \n"
                       "        x_29*x_35 + x_30*x_35 + x_31*x_35 + x_32*x_35 + x_3*x_36 + x_7*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_15*x_36 + x_16*x_36 + x_19*x_36 + x_20*x_36 + x_21*x_36 + x_22*x_36 + x_23*x_36 + \n"
                       "        x_25*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_29*x_36 + x_30*x_36 + x_31*x_36 + x_32*x_36 + x_33*x_36 + x_1*x_37 + x_4*x_37 + x_5*x_37 + x_10*x_37 + x_12*x_37 + x_13*x_37 + x_16*x_37 + \n"
                       "        x_17*x_37 + x_19*x_37 + x_23*x_37 + x_25*x_37 + x_27*x_37 + x_29*x_37 + x_32*x_37 + x_34*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + x_3*x_38 + x_4*x_38 + x_6*x_38 + x_10*x_38 + x_11*x_38 + \n"
                       "        x_12*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_19*x_38 + x_20*x_38 + x_26*x_38 + x_27*x_38 + x_32*x_38 + x_33*x_38 + x_36*x_38 + x_2*x_39 + x_3*x_39 + x_6*x_39 + x_9*x_39 + x_10*x_39 + \n"
                       "        x_11*x_39 + x_13*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_17*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + x_36*x_39 + x_1*x_40 + x_5*x_40 + x_6*x_40 + \n"
                       "        x_7*x_40 + x_9*x_40 + x_10*x_40 + x_11*x_40 + x_13*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_20*x_40 + x_21*x_40 + x_22*x_40 + x_23*x_40 + x_24*x_40 + x_26*x_40 + x_30*x_40 + x_31*x_40 + \n"
                       "        x_32*x_40 + x_35*x_40 + x_36*x_40 + x_37*x_40 + x_39*x_40 + x_2*x_41 + x_3*x_41 + x_7*x_41 + x_9*x_41 + x_10*x_41 + x_11*x_41 + x_12*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + \n"
                       "        x_18*x_41 + x_19*x_41 + x_22*x_41 + x_23*x_41 + x_27*x_41 + x_28*x_41 + x_29*x_41 + x_32*x_41 + x_33*x_41 + x_35*x_41 + x_38*x_41 + x_40*x_41 + x_3*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + \n"
                       "        x_12*x_42 + x_15*x_42 + x_19*x_42 + x_21*x_42 + x_28*x_42 + x_30*x_42 + x_31*x_42 + x_36*x_42 + x_37*x_42 + x_38*x_42 + x_40*x_42 + x_1*x_43 + x_6*x_43 + x_7*x_43 + x_8*x_43 + x_11*x_43 + \n"
                       "        x_13*x_43 + x_15*x_43 + x_16*x_43 + x_17*x_43 + x_20*x_43 + x_25*x_43 + x_31*x_43 + x_32*x_43 + x_34*x_43 + x_37*x_43 + x_41*x_43 + x_42*x_43 + x_3*x_44 + x_4*x_44 + x_6*x_44 + x_7*x_44 + x_8*x_44\n"
                       "        + x_9*x_44 + x_11*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_17*x_44 + x_18*x_44 + x_21*x_44 + x_23*x_44 + x_25*x_44 + x_28*x_44 + x_30*x_44 + x_31*x_44 + x_34*x_44 + x_36*x_44 + x_37*x_44 + \n"
                       "        x_41*x_44 + x_42*x_44 + x_2*x_45 + x_7*x_45 + x_9*x_45 + x_12*x_45 + x_13*x_45 + x_16*x_45 + x_17*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_24*x_45 + x_26*x_45 + x_27*x_45 + \n"
                       "        x_28*x_45 + x_33*x_45 + x_38*x_45 + x_39*x_45 + x_40*x_45 + x_42*x_45 + x_44*x_45 + x_2*x_46 + x_3*x_46 + x_5*x_46 + x_6*x_46 + x_7*x_46 + x_9*x_46 + x_13*x_46 + x_17*x_46 + x_21*x_46 + x_24*x_46 \n"
                       "        + x_25*x_46 + x_26*x_46 + x_29*x_46 + x_30*x_46 + x_31*x_46 + x_35*x_46 + x_36*x_46 + x_37*x_46 + x_38*x_46 + x_39*x_46 + x_40*x_46 + x_42*x_46 + x_44*x_46 + x_45*x_46 + x_2*x_47 + x_4*x_47 + \n"
                       "        x_5*x_47 + x_6*x_47 + x_8*x_47 + x_11*x_47 + x_12*x_47 + x_15*x_47 + x_17*x_47 + x_20*x_47 + x_22*x_47 + x_23*x_47 + x_28*x_47 + x_29*x_47 + x_31*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + \n"
                       "        x_36*x_47 + x_37*x_47 + x_38*x_47 + x_40*x_47 + x_41*x_47 + x_42*x_47 + x_2*x_48 + x_6*x_48 + x_9*x_48 + x_11*x_48 + x_13*x_48 + x_18*x_48 + x_20*x_48 + x_24*x_48 + x_25*x_48 + x_27*x_48 + \n"
                       "        x_29*x_48 + x_30*x_48 + x_32*x_48 + x_33*x_48 + x_36*x_48 + x_39*x_48 + x_40*x_48 + x_42*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_2*x_49 + x_4*x_49 + x_5*x_49 + x_6*x_49 + x_7*x_49\n"
                       "        + x_8*x_49 + x_11*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_17*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49 + x_27*x_49 + x_28*x_49 + x_30*x_49 + \n"
                       "        x_31*x_49 + x_32*x_49 + x_33*x_49 + x_35*x_49 + x_41*x_49 + x_42*x_49 + x_48*x_49 + x_1 + x_2 + x_4 + x_8 + x_9 + x_10 + x_12 + x_16 + x_17 + x_18 + x_22 + x_23 + x_24 + x_30 + x_33 + x_34 + x_35 \n"
                       "        + x_37 + x_38 + x_39 + x_42 + x_45 + x_47 + x_48,\n"
                       "    x_1*x_2 + x_2*x_3 + x_1*x_4 + x_2*x_4 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_1*x_7 + x_2*x_7 + x_3*x_7 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_5*x_8 + x_1*x_9 + x_3*x_9 + x_5*x_9 + x_6*x_9 + x_2*x_10 +\n"
                       "        x_3*x_10 + x_4*x_10 + x_9*x_10 + x_2*x_11 + x_3*x_11 + x_6*x_11 + x_7*x_11 + x_10*x_11 + x_4*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_2*x_13 + x_4*x_13 + \n"
                       "        x_5*x_13 + x_12*x_13 + x_2*x_14 + x_3*x_14 + x_5*x_14 + x_13*x_14 + x_1*x_15 + x_5*x_15 + x_8*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_13*x_15 + x_14*x_15 + x_2*x_16 + x_4*x_16 + x_7*x_16 + \n"
                       "        x_11*x_16 + x_12*x_16 + x_1*x_17 + x_2*x_17 + x_7*x_17 + x_8*x_17 + x_10*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_4*x_18 + x_5*x_18 + x_7*x_18 + x_10*x_18 + x_11*x_18 + \n"
                       "        x_12*x_18 + x_13*x_18 + x_16*x_18 + x_17*x_18 + x_3*x_19 + x_5*x_19 + x_6*x_19 + x_10*x_19 + x_12*x_19 + x_13*x_19 + x_16*x_19 + x_17*x_19 + x_4*x_20 + x_6*x_20 + x_7*x_20 + x_9*x_20 + x_10*x_20 +\n"
                       "        x_11*x_20 + x_13*x_20 + x_15*x_20 + x_18*x_20 + x_1*x_21 + x_2*x_21 + x_3*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_12*x_21 + x_13*x_21 + x_18*x_21 + x_19*x_21 + x_4*x_22 + x_5*x_22 + x_6*x_22 + \n"
                       "        x_7*x_22 + x_8*x_22 + x_10*x_22 + x_11*x_22 + x_13*x_22 + x_15*x_22 + x_17*x_22 + x_20*x_22 + x_1*x_23 + x_7*x_23 + x_8*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_20*x_23 + x_21*x_23 + x_7*x_24 \n"
                       "        + x_8*x_24 + x_11*x_24 + x_13*x_24 + x_16*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_23*x_24 + x_1*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 + x_15*x_25 + \n"
                       "        x_17*x_25 + x_19*x_25 + x_20*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_2*x_26 + x_4*x_26 + x_6*x_26 + x_7*x_26 + x_9*x_26 + x_12*x_26 + x_15*x_26 + x_16*x_26 + x_19*x_26 + x_20*x_26 + x_21*x_26\n"
                       "        + x_23*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_10*x_27 + x_11*x_27 + x_16*x_27 + x_19*x_27 + x_21*x_27 + x_23*x_27 + x_25*x_27 + x_26*x_27 + x_1*x_28 + \n"
                       "        x_2*x_28 + x_3*x_28 + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_7*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + x_15*x_28 + x_17*x_28 + x_18*x_28 + x_19*x_28 + x_26*x_28 +\n"
                       "        x_27*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_5*x_29 + x_6*x_29 + x_12*x_29 + x_16*x_29 + x_21*x_29 + x_22*x_29 + x_3*x_30 + x_4*x_30 + x_5*x_30 + x_7*x_30 + x_10*x_30 + x_13*x_30 + x_15*x_30 + \n"
                       "        x_18*x_30 + x_19*x_30 + x_20*x_30 + x_27*x_30 + x_28*x_30 + x_1*x_31 + x_2*x_31 + x_3*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_13*x_31 + x_14*x_31 + \n"
                       "        x_15*x_31 + x_18*x_31 + x_19*x_31 + x_20*x_31 + x_21*x_31 + x_22*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32 + x_3*x_32 + x_6*x_32 + x_8*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_16*x_32\n"
                       "        + x_17*x_32 + x_18*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32 + x_26*x_32 + x_28*x_32 + x_30*x_32 + x_2*x_33 + x_3*x_33 + x_8*x_33 + x_11*x_33 + x_15*x_33 + x_16*x_33 + x_19*x_33 + x_24*x_33 + \n"
                       "        x_27*x_33 + x_28*x_33 + x_4*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_16*x_34 + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_21*x_34 + x_24*x_34 + \n"
                       "        x_25*x_34 + x_32*x_34 + x_2*x_35 + x_3*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_10*x_35 + x_12*x_35 + x_14*x_35 + x_16*x_35 + x_18*x_35 + x_22*x_35 + x_23*x_35 + x_25*x_35 + x_26*x_35 + x_30*x_35\n"
                       "        + x_31*x_35 + x_33*x_35 + x_34*x_35 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_9*x_36 + x_10*x_36 + x_11*x_36 + x_14*x_36 + x_16*x_36 + x_17*x_36 + x_21*x_36 + x_23*x_36 + x_24*x_36 + x_26*x_36 + \n"
                       "        x_30*x_36 + x_32*x_36 + x_33*x_36 + x_3*x_37 + x_4*x_37 + x_10*x_37 + x_14*x_37 + x_15*x_37 + x_16*x_37 + x_18*x_37 + x_20*x_37 + x_22*x_37 + x_25*x_37 + x_26*x_37 + x_28*x_37 + x_29*x_37 + \n"
                       "        x_30*x_37 + x_31*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + x_36*x_37 + x_1*x_38 + x_2*x_38 + x_4*x_38 + x_5*x_38 + x_7*x_38 + x_10*x_38 + x_11*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_18*x_38\n"
                       "        + x_20*x_38 + x_21*x_38 + x_23*x_38 + x_25*x_38 + x_26*x_38 + x_27*x_38 + x_28*x_38 + x_33*x_38 + x_36*x_38 + x_1*x_39 + x_3*x_39 + x_5*x_39 + x_6*x_39 + x_7*x_39 + x_8*x_39 + x_9*x_39 + x_11*x_39\n"
                       "        + x_12*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_17*x_39 + x_18*x_39 + x_20*x_39 + x_21*x_39 + x_24*x_39 + x_26*x_39 + x_27*x_39 + x_28*x_39 + x_30*x_39 + x_32*x_39 + x_34*x_39 + x_36*x_39 + \n"
                       "        x_37*x_39 + x_38*x_39 + x_2*x_40 + x_6*x_40 + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_13*x_40 + x_15*x_40 + x_17*x_40 + x_19*x_40 + x_21*x_40 + x_23*x_40 + x_25*x_40 + x_28*x_40 + x_29*x_40 + \n"
                       "        x_32*x_40 + x_34*x_40 + x_36*x_40 + x_2*x_41 + x_3*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + x_12*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_18*x_41 + x_22*x_41 + x_24*x_41 + x_26*x_41\n"
                       "        + x_27*x_41 + x_28*x_41 + x_31*x_41 + x_35*x_41 + x_37*x_41 + x_39*x_41 + x_1*x_42 + x_2*x_42 + x_3*x_42 + x_4*x_42 + x_6*x_42 + x_10*x_42 + x_11*x_42 + x_15*x_42 + x_18*x_42 + x_20*x_42 + \n"
                       "        x_21*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_32*x_42 + x_33*x_42 + x_38*x_42 + x_39*x_42 + x_1*x_43 + x_2*x_43 + x_5*x_43 + x_6*x_43 + x_7*x_43 + x_10*x_43 + x_11*x_43 + x_12*x_43 + x_15*x_43\n"
                       "        + x_17*x_43 + x_18*x_43 + x_19*x_43 + x_21*x_43 + x_22*x_43 + x_24*x_43 + x_25*x_43 + x_30*x_43 + x_32*x_43 + x_33*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_40*x_43 + x_8*x_44 + \n"
                       "        x_10*x_44 + x_12*x_44 + x_15*x_44 + x_16*x_44 + x_19*x_44 + x_21*x_44 + x_23*x_44 + x_26*x_44 + x_28*x_44 + x_30*x_44 + x_31*x_44 + x_32*x_44 + x_33*x_44 + x_38*x_44 + x_43*x_44 + x_1*x_45 + \n"
                       "        x_2*x_45 + x_3*x_45 + x_5*x_45 + x_6*x_45 + x_8*x_45 + x_10*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_15*x_45 + x_16*x_45 + x_18*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_24*x_45\n"
                       "        + x_28*x_45 + x_29*x_45 + x_32*x_45 + x_33*x_45 + x_34*x_45 + x_36*x_45 + x_37*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_3*x_46 + x_4*x_46 + x_5*x_46 + x_9*x_46 + x_11*x_46 + x_12*x_46 + \n"
                       "        x_13*x_46 + x_18*x_46 + x_22*x_46 + x_24*x_46 + x_25*x_46 + x_28*x_46 + x_32*x_46 + x_33*x_46 + x_34*x_46 + x_38*x_46 + x_39*x_46 + x_40*x_46 + x_42*x_46 + x_1*x_47 + x_5*x_47 + x_9*x_47 + \n"
                       "        x_10*x_47 + x_11*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_18*x_47 + x_21*x_47 + x_24*x_47 + x_26*x_47 + x_28*x_47 + x_30*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_36*x_47 + \n"
                       "        x_38*x_47 + x_39*x_47 + x_40*x_47 + x_43*x_47 + x_44*x_47 + x_1*x_48 + x_2*x_48 + x_7*x_48 + x_8*x_48 + x_9*x_48 + x_10*x_48 + x_13*x_48 + x_14*x_48 + x_16*x_48 + x_18*x_48 + x_24*x_48 + x_25*x_48\n"
                       "        + x_27*x_48 + x_30*x_48 + x_32*x_48 + x_35*x_48 + x_36*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_43*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_1*x_49 + x_7*x_49 + x_9*x_49 + \n"
                       "        x_11*x_49 + x_12*x_49 + x_14*x_49 + x_15*x_49 + x_17*x_49 + x_18*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49 + x_22*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + \n"
                       "        x_32*x_49 + x_33*x_49 + x_34*x_49 + x_37*x_49 + x_39*x_49 + x_42*x_49 + x_45*x_49 + x_46*x_49 + x_48*x_49 + x_3 + x_4 + x_6 + x_8 + x_12 + x_13 + x_14 + x_16 + x_18 + x_19 + x_20 + x_21 + x_22 + \n"
                       "        x_24 + x_25 + x_26 + x_28 + x_29 + x_31 + x_35 + x_36 + x_38 + x_39 + x_41 + x_43 + x_44 + x_46 + x_48,\n"
                       "    x_1*x_4 + x_2*x_4 + x_1*x_5 + x_2*x_5 + x_4*x_5 + x_3*x_6 + x_4*x_6 + x_1*x_7 + x_2*x_7 + x_5*x_7 + x_6*x_7 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_7*x_9 + x_3*x_10 + x_4*x_10 + x_5*x_10 + \n"
                       "        x_7*x_10 + x_8*x_10 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_8*x_11 + x_9*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_3*x_13 + \n"
                       "        x_4*x_13 + x_5*x_13 + x_3*x_14 + x_5*x_14 + x_6*x_14 + x_10*x_14 + x_12*x_14 + x_13*x_14 + x_1*x_15 + x_4*x_15 + x_6*x_15 + x_7*x_15 + x_9*x_15 + x_10*x_15 + x_12*x_15 + x_1*x_16 + x_4*x_16 + \n"
                       "        x_6*x_16 + x_7*x_16 + x_9*x_16 + x_10*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_1*x_17 + x_2*x_17 + x_6*x_17 + x_11*x_17 + x_12*x_17 + x_15*x_17 + x_16*x_17 + x_2*x_18 + x_6*x_18 + x_8*x_18 + \n"
                       "        x_10*x_18 + x_12*x_18 + x_13*x_18 + x_15*x_18 + x_17*x_18 + x_2*x_19 + x_4*x_19 + x_6*x_19 + x_8*x_19 + x_10*x_19 + x_14*x_19 + x_15*x_19 + x_17*x_19 + x_1*x_20 + x_2*x_20 + x_5*x_20 + x_10*x_20 +\n"
                       "        x_12*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_17*x_20 + x_18*x_20 + x_1*x_21 + x_2*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_9*x_21 + x_13*x_21 + x_15*x_21 + x_18*x_21 + x_19*x_21 +\n"
                       "        x_20*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_6*x_22 + x_8*x_22 + x_9*x_22 + x_12*x_22 + x_14*x_22 + x_15*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_2*x_23 + x_4*x_23 + x_5*x_23 + x_7*x_23 + \n"
                       "        x_9*x_23 + x_14*x_23 + x_19*x_23 + x_20*x_23 + x_22*x_23 + x_1*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_15*x_24 + x_16*x_24 + x_17*x_24 + x_18*x_24 + x_19*x_24 + x_2*x_25 + x_5*x_25 + x_6*x_25\n"
                       "        + x_7*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_13*x_25 + x_14*x_25 + x_15*x_25 + x_22*x_25 + x_4*x_26 + x_5*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_10*x_26 + x_11*x_26 + x_12*x_26 \n"
                       "        + x_15*x_26 + x_16*x_26 + x_18*x_26 + x_20*x_26 + x_22*x_26 + x_25*x_26 + x_2*x_27 + x_5*x_27 + x_6*x_27 + x_12*x_27 + x_13*x_27 + x_14*x_27 + x_17*x_27 + x_18*x_27 + x_20*x_27 + x_21*x_27 + \n"
                       "        x_23*x_27 + x_2*x_28 + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_7*x_28 + x_8*x_28 + x_9*x_28 + x_10*x_28 + x_17*x_28 + x_20*x_28 + x_21*x_28 + x_25*x_28 + x_26*x_28 + x_4*x_29 + x_5*x_29 + x_7*x_29 + \n"
                       "        x_8*x_29 + x_10*x_29 + x_13*x_29 + x_17*x_29 + x_19*x_29 + x_21*x_29 + x_25*x_29 + x_27*x_29 + x_3*x_30 + x_6*x_30 + x_8*x_30 + x_10*x_30 + x_11*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + \n"
                       "        x_19*x_30 + x_20*x_30 + x_21*x_30 + x_22*x_30 + x_23*x_30 + x_1*x_31 + x_2*x_31 + x_4*x_31 + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_15*x_31 + x_17*x_31 + x_20*x_31 + x_21*x_31 + x_23*x_31 + x_24*x_31 \n"
                       "        + x_28*x_31 + x_30*x_31 + x_10*x_32 + x_13*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_22*x_32 + x_26*x_32 + x_28*x_32 + x_30*x_32 + x_2*x_33 + x_4*x_33 + x_5*x_33 + x_6*x_33 + x_8*x_33 + \n"
                       "        x_11*x_33 + x_12*x_33 + x_13*x_33 + x_15*x_33 + x_17*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_24*x_33 + x_27*x_33 + x_28*x_33 + x_30*x_33 + x_31*x_33 + x_32*x_33 + x_2*x_34 + x_3*x_34 + \n"
                       "        x_6*x_34 + x_9*x_34 + x_10*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_23*x_34 + x_25*x_34 + x_27*x_34 + x_1*x_35 + x_2*x_35 + x_5*x_35 + x_6*x_35 \n"
                       "        + x_8*x_35 + x_9*x_35 + x_13*x_35 + x_17*x_35 + x_18*x_35 + x_20*x_35 + x_21*x_35 + x_24*x_35 + x_26*x_35 + x_29*x_35 + x_31*x_35 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_8*x_36 + x_9*x_36 + x_12*x_36\n"
                       "        + x_14*x_36 + x_16*x_36 + x_18*x_36 + x_20*x_36 + x_21*x_36 + x_24*x_36 + x_27*x_36 + x_28*x_36 + x_32*x_36 + x_35*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_8*x_37 + x_9*x_37 + \n"
                       "        x_12*x_37 + x_14*x_37 + x_15*x_37 + x_16*x_37 + x_17*x_37 + x_21*x_37 + x_23*x_37 + x_24*x_37 + x_25*x_37 + x_27*x_37 + x_29*x_37 + x_32*x_37 + x_33*x_37 + x_36*x_37 + x_6*x_38 + x_7*x_38 + \n"
                       "        x_8*x_38 + x_9*x_38 + x_12*x_38 + x_13*x_38 + x_14*x_38 + x_16*x_38 + x_18*x_38 + x_20*x_38 + x_24*x_38 + x_25*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_32*x_38 + \n"
                       "        x_33*x_38 + x_34*x_38 + x_35*x_38 + x_36*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + x_6*x_39 + x_7*x_39 + x_9*x_39 + x_15*x_39 + x_18*x_39 + x_19*x_39 + x_20*x_39 + x_23*x_39 + x_26*x_39 + x_28*x_39\n"
                       "        + x_30*x_39 + x_36*x_39 + x_3*x_40 + x_5*x_40 + x_8*x_40 + x_10*x_40 + x_12*x_40 + x_14*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_20*x_40 + x_21*x_40 + x_24*x_40 + x_26*x_40 + \n"
                       "        x_27*x_40 + x_30*x_40 + x_31*x_40 + x_32*x_40 + x_33*x_40 + x_36*x_40 + x_38*x_40 + x_1*x_41 + x_2*x_41 + x_6*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_16*x_41\n"
                       "        + x_17*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41 + x_25*x_41 + x_27*x_41 + x_28*x_41 + x_30*x_41 + x_32*x_41 + x_33*x_41 + x_37*x_41 + x_39*x_41 + x_3*x_42 + x_11*x_42 + x_13*x_42 + \n"
                       "        x_16*x_42 + x_17*x_42 + x_18*x_42 + x_20*x_42 + x_23*x_42 + x_26*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_36*x_42 + x_38*x_42 + \n"
                       "        x_40*x_42 + x_1*x_43 + x_2*x_43 + x_3*x_43 + x_4*x_43 + x_11*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_18*x_43 + x_19*x_43 + x_21*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + \n"
                       "        x_31*x_43 + x_32*x_43 + x_34*x_43 + x_36*x_43 + x_38*x_43 + x_39*x_43 + x_41*x_43 + x_42*x_43 + x_1*x_44 + x_3*x_44 + x_6*x_44 + x_12*x_44 + x_14*x_44 + x_15*x_44 + x_17*x_44 + x_20*x_44 + \n"
                       "        x_22*x_44 + x_23*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_30*x_44 + x_32*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_37*x_44 + x_39*x_44 + x_40*x_44 + x_42*x_44 + \n"
                       "        x_1*x_45 + x_2*x_45 + x_3*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_10*x_45 + x_11*x_45 + x_12*x_45 + x_15*x_45 + x_17*x_45 + x_19*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_24*x_45 +\n"
                       "        x_25*x_45 + x_27*x_45 + x_28*x_45 + x_33*x_45 + x_34*x_45 + x_36*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_43*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_5*x_46 + x_7*x_46\n"
                       "        + x_8*x_46 + x_9*x_46 + x_10*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + x_16*x_46 + x_21*x_46 + x_27*x_46 + x_29*x_46 + x_30*x_46 + x_31*x_46 + x_32*x_46 + x_33*x_46 + x_37*x_46 + x_39*x_46 + \n"
                       "        x_44*x_46 + x_1*x_47 + x_4*x_47 + x_5*x_47 + x_6*x_47 + x_9*x_47 + x_11*x_47 + x_12*x_47 + x_15*x_47 + x_19*x_47 + x_20*x_47 + x_22*x_47 + x_24*x_47 + x_30*x_47 + x_33*x_47 + x_36*x_47 + x_37*x_47\n"
                       "        + x_39*x_47 + x_40*x_47 + x_42*x_47 + x_44*x_47 + x_45*x_47 + x_46*x_47 + x_1*x_48 + x_2*x_48 + x_3*x_48 + x_5*x_48 + x_6*x_48 + x_8*x_48 + x_9*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_15*x_48\n"
                       "        + x_18*x_48 + x_19*x_48 + x_20*x_48 + x_22*x_48 + x_23*x_48 + x_24*x_48 + x_25*x_48 + x_29*x_48 + x_30*x_48 + x_33*x_48 + x_39*x_48 + x_40*x_48 + x_43*x_48 + x_44*x_48 + x_1*x_49 + x_2*x_49 + \n"
                       "        x_4*x_49 + x_6*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_12*x_49 + x_13*x_49 + x_14*x_49 + x_16*x_49 + x_19*x_49 + x_22*x_49 + x_23*x_49 + x_26*x_49 + x_27*x_49 + x_29*x_49 + x_30*x_49\n"
                       "        + x_32*x_49 + x_34*x_49 + x_35*x_49 + x_38*x_49 + x_39*x_49 + x_41*x_49 + x_42*x_49 + x_43*x_49 + x_44*x_49 + x_45*x_49 + x_47*x_49 + x_2 + x_3 + x_4 + x_7 + x_9 + x_10 + x_12 + x_13 + x_14 + x_15\n"
                       "        + x_18 + x_19 + x_21 + x_23 + x_24 + x_25 + x_26 + x_28 + x_29 + x_30 + x_31 + x_37 + x_41 + x_45 + x_48 + x_49,\n"
                       "    x_1*x_3 + x_3*x_4 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_2*x_6 + x_2*x_7 + x_1*x_8 + x_2*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_8*x_9 + x_3*x_10 + x_4*x_10 + x_7*x_10\n"
                       "        + x_8*x_10 + x_1*x_11 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_9*x_11 + x_10*x_11 + x_1*x_12 + x_3*x_12 + x_6*x_12 + x_8*x_12 + x_9*x_12 + x_1*x_13 + x_5*x_13 + x_6*x_13 + x_8*x_13 + x_10*x_13 + \n"
                       "        x_2*x_14 + x_6*x_14 + x_7*x_14 + x_9*x_14 + x_10*x_14 + x_11*x_14 + x_12*x_14 + x_13*x_14 + x_1*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_8*x_15 + x_11*x_15 + x_13*x_15 + x_1*x_16 + \n"
                       "        x_3*x_16 + x_4*x_16 + x_6*x_16 + x_8*x_16 + x_9*x_16 + x_13*x_16 + x_15*x_16 + x_4*x_17 + x_7*x_17 + x_8*x_17 + x_11*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + x_2*x_18 + x_7*x_18 + \n"
                       "        x_8*x_18 + x_9*x_18 + x_11*x_18 + x_12*x_18 + x_15*x_18 + x_16*x_18 + x_1*x_19 + x_2*x_19 + x_4*x_19 + x_5*x_19 + x_7*x_19 + x_8*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_6*x_20 + x_8*x_20 + \n"
                       "        x_9*x_20 + x_10*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20 + x_1*x_21 + x_2*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_8*x_21 + x_11*x_21 + x_15*x_21 + x_16*x_21 +\n"
                       "        x_17*x_21 + x_20*x_21 + x_3*x_22 + x_7*x_22 + x_8*x_22 + x_9*x_22 + x_10*x_22 + x_11*x_22 + x_17*x_22 + x_18*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_3*x_23 + x_4*x_23 + x_6*x_23 + \n"
                       "        x_7*x_23 + x_8*x_23 + x_11*x_23 + x_13*x_23 + x_14*x_23 + x_15*x_23 + x_22*x_23 + x_2*x_24 + x_3*x_24 + x_5*x_24 + x_6*x_24 + x_13*x_24 + x_17*x_24 + x_18*x_24 + x_19*x_24 + x_23*x_24 + x_1*x_25 +\n"
                       "        x_5*x_25 + x_6*x_25 + x_8*x_25 + x_13*x_25 + x_18*x_25 + x_22*x_25 + x_23*x_25 + x_3*x_26 + x_5*x_26 + x_6*x_26 + x_9*x_26 + x_10*x_26 + x_11*x_26 + x_13*x_26 + x_16*x_26 + x_17*x_26 + x_24*x_26 +\n"
                       "        x_2*x_27 + x_5*x_27 + x_9*x_27 + x_10*x_27 + x_12*x_27 + x_14*x_27 + x_16*x_27 + x_23*x_27 + x_24*x_27 + x_26*x_27 + x_1*x_28 + x_5*x_28 + x_6*x_28 + x_9*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 +\n"
                       "        x_15*x_28 + x_16*x_28 + x_17*x_28 + x_22*x_28 + x_24*x_28 + x_26*x_28 + x_27*x_28 + x_7*x_29 + x_8*x_29 + x_10*x_29 + x_11*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + x_21*x_29 + x_23*x_29 + \n"
                       "        x_24*x_29 + x_26*x_29 + x_1*x_30 + x_3*x_30 + x_7*x_30 + x_8*x_30 + x_10*x_30 + x_11*x_30 + x_15*x_30 + x_16*x_30 + x_19*x_30 + x_22*x_30 + x_23*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + \n"
                       "        x_28*x_30 + x_29*x_30 + x_2*x_31 + x_3*x_31 + x_7*x_31 + x_9*x_31 + x_11*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_21*x_31 + x_23*x_31 + x_26*x_31 + x_27*x_31 + \n"
                       "        x_29*x_31 + x_30*x_31 + x_2*x_32 + x_3*x_32 + x_4*x_32 + x_5*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_12*x_32 + x_14*x_32 + x_16*x_32 + x_17*x_32 + x_21*x_32 + x_24*x_32 +\n"
                       "        x_25*x_32 + x_29*x_32 + x_30*x_32 + x_31*x_32 + x_2*x_33 + x_4*x_33 + x_6*x_33 + x_8*x_33 + x_9*x_33 + x_12*x_33 + x_15*x_33 + x_17*x_33 + x_18*x_33 + x_23*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33\n"
                       "        + x_29*x_33 + x_32*x_33 + x_1*x_34 + x_3*x_34 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 + x_10*x_34 + x_15*x_34 + x_16*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_21*x_34 + x_22*x_34\n"
                       "        + x_26*x_34 + x_31*x_34 + x_33*x_34 + x_1*x_35 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_14*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_19*x_35 + \n"
                       "        x_25*x_35 + x_26*x_35 + x_29*x_35 + x_31*x_35 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_6*x_36 + x_9*x_36 + x_12*x_36 + x_13*x_36 + x_14*x_36 + x_16*x_36 + x_17*x_36 + x_18*x_36 + x_19*x_36 \n"
                       "        + x_20*x_36 + x_21*x_36 + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_32*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_6*x_37 + x_8*x_37 + x_11*x_37 + x_13*x_37 + \n"
                       "        x_14*x_37 + x_15*x_37 + x_20*x_37 + x_23*x_37 + x_24*x_37 + x_25*x_37 + x_26*x_37 + x_27*x_37 + x_29*x_37 + x_31*x_37 + x_33*x_37 + x_2*x_38 + x_5*x_38 + x_9*x_38 + x_11*x_38 + x_12*x_38 + \n"
                       "        x_15*x_38 + x_17*x_38 + x_20*x_38 + x_22*x_38 + x_23*x_38 + x_25*x_38 + x_31*x_38 + x_33*x_38 + x_35*x_38 + x_36*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + x_10*x_39 + x_14*x_39 + x_17*x_39 + \n"
                       "        x_18*x_39 + x_20*x_39 + x_23*x_39 + x_26*x_39 + x_27*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_32*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_4*x_40 + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_8*x_40 \n"
                       "        + x_9*x_40 + x_10*x_40 + x_12*x_40 + x_13*x_40 + x_15*x_40 + x_17*x_40 + x_18*x_40 + x_23*x_40 + x_24*x_40 + x_26*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + x_31*x_40 + x_33*x_40 + x_35*x_40 + \n"
                       "        x_38*x_40 + x_39*x_40 + x_1*x_41 + x_3*x_41 + x_5*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_12*x_41 + x_13*x_41 + x_18*x_41 + x_20*x_41 + x_21*x_41 + x_23*x_41 + x_28*x_41 + x_29*x_41 + x_30*x_41\n"
                       "        + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_35*x_41 + x_36*x_41 + x_40*x_41 + x_1*x_42 + x_3*x_42 + x_5*x_42 + x_7*x_42 + x_8*x_42 + x_12*x_42 + x_13*x_42 + x_16*x_42 + x_19*x_42 + x_20*x_42 + \n"
                       "        x_21*x_42 + x_26*x_42 + x_31*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_35*x_42 + x_38*x_42 + x_41*x_42 + x_2*x_43 + x_3*x_43 + x_4*x_43 + x_10*x_43 + x_11*x_43 + x_15*x_43 + x_16*x_43 + \n"
                       "        x_17*x_43 + x_18*x_43 + x_21*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 + x_32*x_43 + x_33*x_43 + x_35*x_43 + x_38*x_43 + x_39*x_43 + x_40*x_43 + x_2*x_44 + x_4*x_44 + x_5*x_44 + x_8*x_44 + \n"
                       "        x_11*x_44 + x_12*x_44 + x_13*x_44 + x_15*x_44 + x_19*x_44 + x_21*x_44 + x_23*x_44 + x_24*x_44 + x_25*x_44 + x_28*x_44 + x_29*x_44 + x_33*x_44 + x_35*x_44 + x_38*x_44 + x_41*x_44 + x_42*x_44 + \n"
                       "        x_4*x_45 + x_7*x_45 + x_8*x_45 + x_11*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_22*x_45 + x_24*x_45 + x_25*x_45 + x_31*x_45 + x_33*x_45 + x_34*x_45 + x_36*x_45 + x_37*x_45 + \n"
                       "        x_39*x_45 + x_40*x_45 + x_41*x_45 + x_44*x_45 + x_2*x_46 + x_4*x_46 + x_6*x_46 + x_7*x_46 + x_8*x_46 + x_9*x_46 + x_12*x_46 + x_13*x_46 + x_15*x_46 + x_16*x_46 + x_17*x_46 + x_18*x_46 + x_20*x_46 \n"
                       "        + x_22*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_34*x_46 + x_36*x_46 + x_37*x_46 + x_43*x_46 + x_44*x_46 + x_45*x_46 + x_3*x_47 + x_8*x_47 + x_10*x_47 + \n"
                       "        x_11*x_47 + x_13*x_47 + x_14*x_47 + x_15*x_47 + x_16*x_47 + x_19*x_47 + x_25*x_47 + x_26*x_47 + x_27*x_47 + x_30*x_47 + x_34*x_47 + x_35*x_47 + x_36*x_47 + x_37*x_47 + x_40*x_47 + x_41*x_47 + \n"
                       "        x_44*x_47 + x_1*x_48 + x_3*x_48 + x_4*x_48 + x_5*x_48 + x_6*x_48 + x_7*x_48 + x_8*x_48 + x_11*x_48 + x_18*x_48 + x_21*x_48 + x_27*x_48 + x_28*x_48 + x_30*x_48 + x_33*x_48 + x_35*x_48 + x_36*x_48 +\n"
                       "        x_39*x_48 + x_40*x_48 + x_41*x_48 + x_43*x_48 + x_1*x_49 + x_2*x_49 + x_3*x_49 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_16*x_49 + x_23*x_49 + x_25*x_49 + x_27*x_49 + x_28*x_49 + \n"
                       "        x_30*x_49 + x_35*x_49 + x_36*x_49 + x_38*x_49 + x_39*x_49 + x_44*x_49 + x_45*x_49 + x_47*x_49 + x_3 + x_4 + x_5 + x_8 + x_9 + x_12 + x_13 + x_14 + x_15 + x_16 + x_18 + x_20 + x_27 + x_28 + x_31 + \n"
                       "        x_33 + x_36 + x_39 + x_43 + x_47 + x_49,\n"
                       "    x_1*x_2 + x_1*x_4 + x_1*x_5 + x_1*x_6 + x_3*x_6 + x_4*x_6 + x_2*x_7 + x_3*x_7 + x_6*x_7 + x_4*x_8 + x_5*x_8 + x_6*x_8 + x_4*x_9 + x_6*x_9 + x_7*x_9 + x_1*x_10 + x_7*x_10 + x_3*x_11 + x_4*x_11 + \n"
                       "        x_5*x_11 + x_6*x_11 + x_7*x_11 + x_9*x_11 + x_10*x_11 + x_1*x_12 + x_2*x_12 + x_4*x_12 + x_5*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_4*x_13 + x_6*x_13 + x_8*x_13 + x_3*x_14 + x_4*x_14 + \n"
                       "        x_5*x_14 + x_8*x_14 + x_10*x_14 + x_12*x_14 + x_3*x_15 + x_4*x_15 + x_6*x_15 + x_9*x_15 + x_12*x_15 + x_13*x_15 + x_1*x_16 + x_2*x_16 + x_6*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_12*x_16 + \n"
                       "        x_13*x_16 + x_14*x_16 + x_3*x_17 + x_4*x_17 + x_7*x_17 + x_12*x_17 + x_16*x_17 + x_3*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_9*x_18 + x_11*x_18 + x_12*x_18 + x_14*x_18 + x_15*x_18 + x_16*x_18 + \n"
                       "        x_1*x_19 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_6*x_19 + x_8*x_19 + x_9*x_19 + x_10*x_19 + x_11*x_19 + x_14*x_19 + x_15*x_19 + x_17*x_19 + x_18*x_19 + x_2*x_20 + x_3*x_20 + x_4*x_20 + x_7*x_20 + \n"
                       "        x_9*x_20 + x_11*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 + x_18*x_20 + x_2*x_21 + x_3*x_21 + x_4*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_11*x_21 + x_12*x_21 + x_13*x_21 + x_15*x_21 +\n"
                       "        x_16*x_21 + x_19*x_21 + x_2*x_22 + x_7*x_22 + x_8*x_22 + x_9*x_22 + x_10*x_22 + x_11*x_22 + x_12*x_22 + x_13*x_22 + x_16*x_22 + x_17*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_3*x_23 \n"
                       "        + x_4*x_23 + x_5*x_23 + x_8*x_23 + x_13*x_23 + x_14*x_23 + x_18*x_23 + x_19*x_23 + x_21*x_23 + x_5*x_24 + x_6*x_24 + x_7*x_24 + x_10*x_24 + x_14*x_24 + x_15*x_24 + x_17*x_24 + x_19*x_24 + \n"
                       "        x_22*x_24 + x_23*x_24 + x_1*x_25 + x_2*x_25 + x_3*x_25 + x_4*x_25 + x_5*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 + x_13*x_25 + x_14*x_25 + x_17*x_25 + x_19*x_25 + x_22*x_25 +\n"
                       "        x_24*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_5*x_26 + x_6*x_26 + x_11*x_26 + x_14*x_26 + x_16*x_26 + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_25*x_26 + x_2*x_27 + x_4*x_27 + x_6*x_27 + x_7*x_27 + \n"
                       "        x_8*x_27 + x_11*x_27 + x_12*x_27 + x_13*x_27 + x_15*x_27 + x_16*x_27 + x_19*x_27 + x_22*x_27 + x_23*x_27 + x_25*x_27 + x_26*x_27 + x_1*x_28 + x_3*x_28 + x_6*x_28 + x_7*x_28 + x_8*x_28 + x_9*x_28 +\n"
                       "        x_12*x_28 + x_13*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_21*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_26*x_28 + x_3*x_29 + x_4*x_29 + x_6*x_29 + x_7*x_29 + x_15*x_29 + x_18*x_29 + \n"
                       "        x_19*x_29 + x_22*x_29 + x_24*x_29 + x_26*x_29 + x_28*x_29 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + x_13*x_30 + x_14*x_30 + x_18*x_30 + x_20*x_30 + x_21*x_30 + \n"
                       "        x_27*x_30 + x_1*x_31 + x_9*x_31 + x_10*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_18*x_31 + x_19*x_31 + x_21*x_31 + x_23*x_31 + x_24*x_31 + x_25*x_31 + x_26*x_31 + x_28*x_31 + x_29*x_31 + \n"
                       "        x_30*x_31 + x_1*x_32 + x_3*x_32 + x_9*x_32 + x_10*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_15*x_32 + x_16*x_32 + x_17*x_32 + x_20*x_32 + x_21*x_32 + x_26*x_32 + x_28*x_32 + x_1*x_33 + x_2*x_33\n"
                       "        + x_3*x_33 + x_5*x_33 + x_8*x_33 + x_9*x_33 + x_11*x_33 + x_16*x_33 + x_17*x_33 + x_18*x_33 + x_20*x_33 + x_21*x_33 + x_23*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_29*x_33 + x_31*x_33 + \n"
                       "        x_32*x_33 + x_1*x_34 + x_3*x_34 + x_4*x_34 + x_9*x_34 + x_12*x_34 + x_14*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_25*x_34 + x_28*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_1*x_35 + x_3*x_35 \n"
                       "        + x_4*x_35 + x_7*x_35 + x_8*x_35 + x_10*x_35 + x_11*x_35 + x_12*x_35 + x_13*x_35 + x_18*x_35 + x_22*x_35 + x_24*x_35 + x_27*x_35 + x_28*x_35 + x_29*x_35 + x_30*x_35 + x_32*x_35 + x_1*x_36 + \n"
                       "        x_2*x_36 + x_5*x_36 + x_9*x_36 + x_10*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_16*x_36 + x_17*x_36 + x_19*x_36 + x_21*x_36 + x_22*x_36 + x_26*x_36 + x_30*x_36 + x_31*x_36 + x_33*x_36 + \n"
                       "        x_34*x_36 + x_35*x_36 + x_5*x_37 + x_12*x_37 + x_17*x_37 + x_18*x_37 + x_19*x_37 + x_21*x_37 + x_22*x_37 + x_23*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_2*x_38 + x_3*x_38 + \n"
                       "        x_4*x_38 + x_5*x_38 + x_6*x_38 + x_13*x_38 + x_14*x_38 + x_15*x_38 + x_17*x_38 + x_19*x_38 + x_20*x_38 + x_26*x_38 + x_31*x_38 + x_32*x_38 + x_34*x_38 + x_35*x_38 + x_37*x_38 + x_1*x_39 + x_3*x_39\n"
                       "        + x_5*x_39 + x_7*x_39 + x_9*x_39 + x_10*x_39 + x_12*x_39 + x_13*x_39 + x_15*x_39 + x_16*x_39 + x_19*x_39 + x_20*x_39 + x_22*x_39 + x_23*x_39 + x_25*x_39 + x_27*x_39 + x_28*x_39 + x_32*x_39 + \n"
                       "        x_34*x_39 + x_36*x_39 + x_37*x_39 + x_38*x_39 + x_1*x_40 + x_4*x_40 + x_5*x_40 + x_7*x_40 + x_9*x_40 + x_11*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_17*x_40 + x_18*x_40 + x_19*x_40 + x_21*x_40\n"
                       "        + x_23*x_40 + x_24*x_40 + x_26*x_40 + x_29*x_40 + x_30*x_40 + x_31*x_40 + x_32*x_40 + x_33*x_40 + x_35*x_40 + x_38*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_6*x_41 + x_8*x_41 + x_9*x_41 + \n"
                       "        x_10*x_41 + x_12*x_41 + x_13*x_41 + x_15*x_41 + x_20*x_41 + x_21*x_41 + x_23*x_41 + x_25*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_33*x_41 + x_34*x_41 + x_36*x_41 + x_37*x_41 + x_39*x_41 + \n"
                       "        x_1*x_42 + x_5*x_42 + x_8*x_42 + x_9*x_42 + x_11*x_42 + x_12*x_42 + x_13*x_42 + x_15*x_42 + x_20*x_42 + x_22*x_42 + x_24*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + \n"
                       "        x_35*x_42 + x_36*x_42 + x_38*x_42 + x_4*x_43 + x_6*x_43 + x_7*x_43 + x_11*x_43 + x_12*x_43 + x_13*x_43 + x_15*x_43 + x_16*x_43 + x_18*x_43 + x_19*x_43 + x_20*x_43 + x_21*x_43 + x_24*x_43 + \n"
                       "        x_25*x_43 + x_27*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_34*x_43 + x_35*x_43 + x_36*x_43 + x_38*x_43 + x_39*x_43 + x_42*x_43 + x_1*x_44 + x_3*x_44 + x_5*x_44 + \n"
                       "        x_6*x_44 + x_8*x_44 + x_12*x_44 + x_15*x_44 + x_16*x_44 + x_23*x_44 + x_24*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44 + x_29*x_44 + x_31*x_44 + x_33*x_44 + x_35*x_44 + x_38*x_44 + x_40*x_44 + \n"
                       "        x_42*x_44 + x_2*x_45 + x_5*x_45 + x_6*x_45 + x_8*x_45 + x_10*x_45 + x_12*x_45 + x_13*x_45 + x_14*x_45 + x_18*x_45 + x_19*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_24*x_45 + x_25*x_45 + \n"
                       "        x_27*x_45 + x_31*x_45 + x_34*x_45 + x_35*x_45 + x_36*x_45 + x_37*x_45 + x_39*x_45 + x_42*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_10*x_46 + x_11*x_46 + x_13*x_46 + x_17*x_46 + x_18*x_46 + \n"
                       "        x_19*x_46 + x_20*x_46 + x_21*x_46 + x_27*x_46 + x_30*x_46 + x_32*x_46 + x_33*x_46 + x_34*x_46 + x_37*x_46 + x_43*x_46 + x_44*x_46 + x_2*x_47 + x_5*x_47 + x_7*x_47 + x_9*x_47 + x_13*x_47 + \n"
                       "        x_14*x_47 + x_15*x_47 + x_17*x_47 + x_19*x_47 + x_20*x_47 + x_22*x_47 + x_23*x_47 + x_26*x_47 + x_27*x_47 + x_30*x_47 + x_31*x_47 + x_34*x_47 + x_39*x_47 + x_40*x_47 + x_42*x_47 + x_44*x_47 + \n"
                       "        x_45*x_47 + x_3*x_48 + x_4*x_48 + x_5*x_48 + x_6*x_48 + x_9*x_48 + x_10*x_48 + x_11*x_48 + x_14*x_48 + x_18*x_48 + x_20*x_48 + x_24*x_48 + x_25*x_48 + x_26*x_48 + x_30*x_48 + x_33*x_48 + x_34*x_48\n"
                       "        + x_35*x_48 + x_41*x_48 + x_42*x_48 + x_44*x_48 + x_46*x_48 + x_5*x_49 + x_7*x_49 + x_11*x_49 + x_15*x_49 + x_18*x_49 + x_24*x_49 + x_28*x_49 + x_30*x_49 + x_31*x_49 + x_34*x_49 + x_35*x_49 + \n"
                       "        x_38*x_49 + x_39*x_49 + x_41*x_49 + x_42*x_49 + x_46*x_49 + x_48*x_49 + x_2 + x_4 + x_6 + x_7 + x_8 + x_11 + x_12 + x_13 + x_15 + x_16 + x_17 + x_24 + x_25 + x_26 + x_29 + x_30 + x_32 + x_34 + \n"
                       "        x_37 + x_39 + x_41 + x_43 + x_44 + x_48 + x_49,\n"
                       "    x_1*x_2 + x_1*x_4 + x_1*x_5 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_4*x_6 + x_5*x_6 + x_3*x_7 + x_4*x_7 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_2*x_9 + x_6*x_9 + x_2*x_10 + x_3*x_10 \n"
                       "        + x_4*x_10 + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_9*x_10 + x_1*x_11 + x_3*x_11 + x_5*x_11 + x_6*x_11 + x_7*x_11 + x_9*x_11 + x_10*x_11 + x_1*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_8*x_12 + \n"
                       "        x_9*x_12 + x_10*x_12 + x_11*x_12 + x_2*x_13 + x_3*x_13 + x_6*x_13 + x_8*x_13 + x_9*x_13 + x_10*x_13 + x_11*x_13 + x_2*x_14 + x_3*x_14 + x_4*x_14 + x_6*x_14 + x_7*x_14 + x_10*x_14 + x_11*x_14 + \n"
                       "        x_2*x_15 + x_3*x_15 + x_4*x_15 + x_6*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_1*x_16 + x_2*x_16 + x_5*x_16 + x_10*x_16 + x_13*x_16 + x_1*x_17 + x_3*x_17 + x_5*x_17 + x_6*x_17 + x_7*x_17 + \n"
                       "        x_8*x_17 + x_10*x_17 + x_11*x_17 + x_12*x_17 + x_13*x_17 + x_14*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_13*x_18 + \n"
                       "        x_14*x_18 + x_1*x_19 + x_3*x_19 + x_4*x_19 + x_6*x_19 + x_7*x_19 + x_8*x_19 + x_10*x_19 + x_12*x_19 + x_13*x_19 + x_16*x_19 + x_17*x_19 + x_1*x_20 + x_3*x_20 + x_4*x_20 + x_6*x_20 + x_8*x_20 + \n"
                       "        x_9*x_20 + x_11*x_20 + x_12*x_20 + x_13*x_20 + x_17*x_20 + x_18*x_20 + x_19*x_20 + x_1*x_21 + x_2*x_21 + x_4*x_21 + x_5*x_21 + x_8*x_21 + x_12*x_21 + x_14*x_21 + x_15*x_21 + x_2*x_22 + x_3*x_22 + \n"
                       "        x_4*x_22 + x_5*x_22 + x_6*x_22 + x_8*x_22 + x_12*x_22 + x_13*x_22 + x_14*x_22 + x_15*x_22 + x_18*x_22 + x_21*x_22 + x_1*x_23 + x_2*x_23 + x_5*x_23 + x_6*x_23 + x_7*x_23 + x_10*x_23 + x_13*x_23 + \n"
                       "        x_14*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23 + x_22*x_23 + x_1*x_24 + x_2*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_13*x_24 + x_16*x_24 + x_18*x_24 + x_22*x_24 + x_23*x_24\n"
                       "        + x_3*x_25 + x_7*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 + x_12*x_25 + x_13*x_25 + x_15*x_25 + x_19*x_25 + x_20*x_25 + x_21*x_25 + x_22*x_25 + x_4*x_26 + x_8*x_26 + x_12*x_26 + x_13*x_26 + \n"
                       "        x_14*x_26 + x_16*x_26 + x_18*x_26 + x_19*x_26 + x_20*x_26 + x_21*x_26 + x_2*x_27 + x_3*x_27 + x_4*x_27 + x_5*x_27 + x_9*x_27 + x_10*x_27 + x_12*x_27 + x_23*x_27 + x_26*x_27 + x_1*x_28 + x_3*x_28 +\n"
                       "        x_6*x_28 + x_7*x_28 + x_9*x_28 + x_11*x_28 + x_13*x_28 + x_15*x_28 + x_16*x_28 + x_18*x_28 + x_23*x_28 + x_25*x_28 + x_26*x_28 + x_1*x_29 + x_13*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + \n"
                       "        x_18*x_29 + x_24*x_29 + x_28*x_29 + x_1*x_30 + x_5*x_30 + x_7*x_30 + x_8*x_30 + x_10*x_30 + x_13*x_30 + x_14*x_30 + x_16*x_30 + x_18*x_30 + x_19*x_30 + x_20*x_30 + x_22*x_30 + x_24*x_30 + \n"
                       "        x_25*x_30 + x_1*x_31 + x_2*x_31 + x_3*x_31 + x_4*x_31 + x_5*x_31 + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_21*x_31 + x_22*x_31 + \n"
                       "        x_23*x_31 + x_24*x_31 + x_25*x_31 + x_27*x_31 + x_2*x_32 + x_3*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_12*x_32 + x_14*x_32 + x_15*x_32 + x_16*x_32 + x_17*x_32 + x_18*x_32 + x_21*x_32 + \n"
                       "        x_22*x_32 + x_23*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_31*x_32 + x_1*x_33 + x_4*x_33 + x_7*x_33 + x_12*x_33 + x_13*x_33 + x_14*x_33 + x_19*x_33 + x_20*x_33 + \n"
                       "        x_25*x_33 + x_26*x_33 + x_28*x_33 + x_29*x_33 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_10*x_34 + x_13*x_34 + x_15*x_34 + x_18*x_34 + x_19*x_34 + x_22*x_34 + x_23*x_34 + x_26*x_34 + x_28*x_34 + \n"
                       "        x_30*x_34 + x_31*x_34 + x_32*x_34 + x_1*x_35 + x_3*x_35 + x_5*x_35 + x_9*x_35 + x_11*x_35 + x_12*x_35 + x_14*x_35 + x_15*x_35 + x_16*x_35 + x_17*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35 + \n"
                       "        x_25*x_35 + x_31*x_35 + x_33*x_35 + x_1*x_36 + x_2*x_36 + x_4*x_36 + x_5*x_36 + x_6*x_36 + x_8*x_36 + x_10*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_18*x_36 + x_22*x_36 + x_23*x_36 + x_27*x_36 \n"
                       "        + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_12*x_37 + x_14*x_37 + x_15*x_37 + x_16*x_37 + x_22*x_37 + x_24*x_37 + x_26*x_37 + x_27*x_37\n"
                       "        + x_30*x_37 + x_31*x_37 + x_32*x_37 + x_33*x_37 + x_35*x_37 + x_1*x_38 + x_3*x_38 + x_4*x_38 + x_6*x_38 + x_7*x_38 + x_9*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_14*x_38 + x_15*x_38 + \n"
                       "        x_16*x_38 + x_18*x_38 + x_25*x_38 + x_30*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_34*x_38 + x_2*x_39 + x_5*x_39 + x_7*x_39 + x_9*x_39 + x_11*x_39 + x_12*x_39 + x_14*x_39 + x_15*x_39 + \n"
                       "        x_16*x_39 + x_20*x_39 + x_24*x_39 + x_25*x_39 + x_26*x_39 + x_29*x_39 + x_31*x_39 + x_35*x_39 + x_36*x_39 + x_2*x_40 + x_4*x_40 + x_6*x_40 + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_13*x_40 + x_15*x_40\n"
                       "        + x_17*x_40 + x_18*x_40 + x_21*x_40 + x_23*x_40 + x_25*x_40 + x_27*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + x_32*x_40 + x_33*x_40 + x_34*x_40 + x_35*x_40 + x_36*x_40 + x_38*x_40 + x_3*x_41 + \n"
                       "        x_4*x_41 + x_9*x_41 + x_10*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41 + x_27*x_41 + x_28*x_41 + x_29*x_41 + x_40*x_41 + x_1*x_42 + x_2*x_42 + x_4*x_42\n"
                       "        + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_8*x_42 + x_9*x_42 + x_10*x_42 + x_12*x_42 + x_13*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_25*x_42 + x_26*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + \n"
                       "        x_37*x_42 + x_38*x_42 + x_40*x_42 + x_1*x_43 + x_2*x_43 + x_3*x_43 + x_6*x_43 + x_9*x_43 + x_11*x_43 + x_13*x_43 + x_15*x_43 + x_17*x_43 + x_18*x_43 + x_21*x_43 + x_22*x_43 + x_29*x_43 + x_31*x_43\n"
                       "        + x_32*x_43 + x_35*x_43 + x_37*x_43 + x_39*x_43 + x_40*x_43 + x_41*x_43 + x_42*x_43 + x_5*x_44 + x_7*x_44 + x_9*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_14*x_44 + x_15*x_44 + x_16*x_44 + \n"
                       "        x_17*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_28*x_44 + x_34*x_44 + x_38*x_44 + x_39*x_44 + x_41*x_44 + x_42*x_44 + x_43*x_44 + x_4*x_45 + x_7*x_45 + x_8*x_45 + x_9*x_45 + \n"
                       "        x_10*x_45 + x_13*x_45 + x_14*x_45 + x_16*x_45 + x_18*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_25*x_45 + x_26*x_45 + x_27*x_45 + x_28*x_45 + x_30*x_45 + x_31*x_45 + x_32*x_45 + x_40*x_45 + \n"
                       "        x_44*x_45 + x_3*x_46 + x_8*x_46 + x_9*x_46 + x_11*x_46 + x_12*x_46 + x_14*x_46 + x_15*x_46 + x_17*x_46 + x_18*x_46 + x_19*x_46 + x_21*x_46 + x_23*x_46 + x_24*x_46 + x_29*x_46 + x_30*x_46 + \n"
                       "        x_35*x_46 + x_36*x_46 + x_39*x_46 + x_42*x_46 + x_45*x_46 + x_1*x_47 + x_2*x_47 + x_3*x_47 + x_4*x_47 + x_5*x_47 + x_7*x_47 + x_8*x_47 + x_11*x_47 + x_12*x_47 + x_14*x_47 + x_15*x_47 + x_18*x_47 +\n"
                       "        x_19*x_47 + x_20*x_47 + x_22*x_47 + x_25*x_47 + x_30*x_47 + x_33*x_47 + x_35*x_47 + x_36*x_47 + x_39*x_47 + x_41*x_47 + x_42*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_1*x_48 + x_2*x_48 + \n"
                       "        x_5*x_48 + x_6*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_14*x_48 + x_18*x_48 + x_19*x_48 + x_22*x_48 + x_24*x_48 + x_25*x_48 + x_27*x_48 + x_30*x_48 + x_31*x_48 + x_32*x_48 + \n"
                       "        x_35*x_48 + x_37*x_48 + x_39*x_48 + x_43*x_48 + x_45*x_48 + x_47*x_48 + x_2*x_49 + x_3*x_49 + x_6*x_49 + x_8*x_49 + x_10*x_49 + x_11*x_49 + x_13*x_49 + x_14*x_49 + x_17*x_49 + x_18*x_49 + \n"
                       "        x_20*x_49 + x_21*x_49 + x_22*x_49 + x_25*x_49 + x_27*x_49 + x_29*x_49 + x_31*x_49 + x_32*x_49 + x_33*x_49 + x_38*x_49 + x_39*x_49 + x_41*x_49 + x_43*x_49 + x_44*x_49 + x_46*x_49 + x_4 + x_6 + x_7 \n"
                       "        + x_8 + x_9 + x_10 + x_11 + x_13 + x_14 + x_15 + x_17 + x_18 + x_19 + x_20 + x_21 + x_23 + x_24 + x_27 + x_30 + x_32 + x_33 + x_35 + x_39 + x_40 + x_42 + x_45,\n"
                       "    x_1*x_2 + x_2*x_3 + x_1*x_4 + x_3*x_4 + x_4*x_5 + x_1*x_6 + x_4*x_6 + x_1*x_7 + x_2*x_7 + x_3*x_7 + x_4*x_7 + x_6*x_7 + x_2*x_8 + x_3*x_8 + x_4*x_8 + x_5*x_8 + x_6*x_8 + x_3*x_9 + x_8*x_9 + x_2*x_10 +\n"
                       "        x_3*x_10 + x_4*x_10 + x_6*x_10 + x_9*x_10 + x_2*x_11 + x_5*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_1*x_12 + x_10*x_12 + x_8*x_13 + x_9*x_13 + x_2*x_14 + x_3*x_14 + x_5*x_14 + x_6*x_14 + \n"
                       "        x_7*x_14 + x_8*x_14 + x_11*x_14 + x_7*x_15 + x_13*x_15 + x_14*x_15 + x_1*x_16 + x_4*x_16 + x_5*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_2*x_17 + x_4*x_17 + \n"
                       "        x_7*x_17 + x_8*x_17 + x_9*x_17 + x_13*x_17 + x_14*x_17 + x_1*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_6*x_18 + x_8*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + \n"
                       "        x_5*x_19 + x_6*x_19 + x_11*x_19 + x_14*x_19 + x_15*x_19 + x_18*x_19 + x_3*x_20 + x_4*x_20 + x_6*x_20 + x_8*x_20 + x_9*x_20 + x_11*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 + x_18*x_20 + x_19*x_20 +\n"
                       "        x_6*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_13*x_21 + x_14*x_21 + x_17*x_21 + x_20*x_21 + x_1*x_22 + x_2*x_22 + x_5*x_22 + x_13*x_22 + x_14*x_22 + x_15*x_22 + x_19*x_22 + x_21*x_22 +\n"
                       "        x_1*x_23 + x_2*x_23 + x_4*x_23 + x_5*x_23 + x_8*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_18*x_23 + x_19*x_23 + x_21*x_23 + x_22*x_23 + x_3*x_24 + x_4*x_24 + x_13*x_24 + x_15*x_24 + x_18*x_24 +\n"
                       "        x_19*x_24 + x_22*x_24 + x_23*x_24 + x_5*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_13*x_25 + x_15*x_25 + x_16*x_25 + x_17*x_25 + x_20*x_25 + x_24*x_25 + x_1*x_26 + x_2*x_26 + x_9*x_26 + x_11*x_26\n"
                       "        + x_12*x_26 + x_14*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_19*x_26 + x_21*x_26 + x_22*x_26 + x_24*x_26 + x_25*x_26 + x_2*x_27 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_9*x_27 + x_12*x_27 + \n"
                       "        x_13*x_27 + x_16*x_27 + x_17*x_27 + x_21*x_27 + x_24*x_27 + x_26*x_27 + x_1*x_28 + x_2*x_28 + x_4*x_28 + x_6*x_28 + x_8*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 + x_12*x_28 + x_13*x_28 + x_15*x_28 \n"
                       "        + x_16*x_28 + x_19*x_28 + x_21*x_28 + x_23*x_28 + x_24*x_28 + x_26*x_28 + x_27*x_28 + x_2*x_29 + x_4*x_29 + x_6*x_29 + x_7*x_29 + x_8*x_29 + x_11*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + \n"
                       "        x_20*x_29 + x_25*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_5*x_30 + x_8*x_30 + x_9*x_30 + x_11*x_30 + x_12*x_30 + x_14*x_30 + x_15*x_30 + x_19*x_30 + x_24*x_30 + x_25*x_30 + x_27*x_30 + \n"
                       "        x_28*x_30 + x_4*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 + x_12*x_31 + x_13*x_31 + x_15*x_31 + x_18*x_31 + x_20*x_31 + x_22*x_31 + x_25*x_31 + x_26*x_31 + x_28*x_31 + x_29*x_31 + x_1*x_32 + \n"
                       "        x_2*x_32 + x_3*x_32 + x_4*x_32 + x_7*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_12*x_32 + x_14*x_32 + x_16*x_32 + x_17*x_32 + x_20*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32\n"
                       "        + x_29*x_32 + x_30*x_32 + x_7*x_33 + x_9*x_33 + x_12*x_33 + x_13*x_33 + x_16*x_33 + x_17*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_26*x_33 + x_27*x_33 + x_29*x_33 + x_30*x_33 + x_31*x_33 + \n"
                       "        x_32*x_33 + x_1*x_34 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_5*x_34 + x_7*x_34 + x_9*x_34 + x_10*x_34 + x_16*x_34 + x_17*x_34 + x_18*x_34 + x_23*x_34 + x_24*x_34 + x_25*x_34 + x_29*x_34 + x_32*x_34 +\n"
                       "        x_1*x_35 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_10*x_35 + x_14*x_35 + x_15*x_35 + x_18*x_35 + x_19*x_35 + x_20*x_35 + x_23*x_35 + x_29*x_35 + x_30*x_35 + x_31*x_35 +\n"
                       "        x_33*x_35 + x_34*x_35 + x_1*x_36 + x_3*x_36 + x_4*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 + x_9*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_14*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + x_21*x_36 +\n"
                       "        x_23*x_36 + x_24*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_31*x_36 + x_32*x_36 + x_34*x_36 + x_2*x_37 + x_4*x_37 + x_7*x_37 + x_10*x_37 + x_12*x_37 + x_14*x_37 + x_16*x_37 + x_17*x_37 + \n"
                       "        x_18*x_37 + x_20*x_37 + x_21*x_37 + x_22*x_37 + x_23*x_37 + x_24*x_37 + x_25*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_31*x_37 + x_33*x_37 + x_34*x_37 + x_35*x_37 + x_1*x_38 + x_2*x_38 + \n"
                       "        x_6*x_38 + x_8*x_38 + x_9*x_38 + x_10*x_38 + x_12*x_38 + x_14*x_38 + x_15*x_38 + x_18*x_38 + x_20*x_38 + x_22*x_38 + x_25*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_36*x_38 + \n"
                       "        x_37*x_38 + x_4*x_39 + x_6*x_39 + x_8*x_39 + x_12*x_39 + x_16*x_39 + x_19*x_39 + x_21*x_39 + x_29*x_39 + x_30*x_39 + x_32*x_39 + x_34*x_39 + x_35*x_39 + x_38*x_39 + x_1*x_40 + x_4*x_40 + x_6*x_40 \n"
                       "        + x_7*x_40 + x_8*x_40 + x_9*x_40 + x_10*x_40 + x_13*x_40 + x_16*x_40 + x_17*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + x_22*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_28*x_40 + x_29*x_40 + \n"
                       "        x_30*x_40 + x_31*x_40 + x_33*x_40 + x_35*x_40 + x_38*x_40 + x_39*x_40 + x_2*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + \n"
                       "        x_17*x_41 + x_18*x_41 + x_19*x_41 + x_21*x_41 + x_28*x_41 + x_30*x_41 + x_31*x_41 + x_33*x_41 + x_39*x_41 + x_3*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_9*x_42 + x_11*x_42 + x_12*x_42 + x_13*x_42\n"
                       "        + x_16*x_42 + x_17*x_42 + x_20*x_42 + x_22*x_42 + x_25*x_42 + x_26*x_42 + x_27*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_37*x_42 + x_39*x_42 + x_40*x_42 + x_41*x_42 + \n"
                       "        x_2*x_43 + x_3*x_43 + x_5*x_43 + x_6*x_43 + x_7*x_43 + x_8*x_43 + x_9*x_43 + x_10*x_43 + x_12*x_43 + x_13*x_43 + x_16*x_43 + x_21*x_43 + x_25*x_43 + x_27*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 +\n"
                       "        x_33*x_43 + x_34*x_43 + x_36*x_43 + x_39*x_43 + x_40*x_43 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_6*x_44 + x_7*x_44 + x_10*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_18*x_44 + x_20*x_44 + x_21*x_44\n"
                       "        + x_22*x_44 + x_24*x_44 + x_27*x_44 + x_29*x_44 + x_30*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_39*x_44 + x_40*x_44 + x_42*x_44 + x_2*x_45 + x_3*x_45 + x_4*x_45 + x_5*x_45 + \n"
                       "        x_6*x_45 + x_7*x_45 + x_8*x_45 + x_11*x_45 + x_16*x_45 + x_17*x_45 + x_19*x_45 + x_23*x_45 + x_24*x_45 + x_25*x_45 + x_26*x_45 + x_27*x_45 + x_28*x_45 + x_29*x_45 + x_30*x_45 + x_33*x_45 + \n"
                       "        x_34*x_45 + x_38*x_45 + x_40*x_45 + x_41*x_45 + x_43*x_45 + x_2*x_46 + x_4*x_46 + x_5*x_46 + x_8*x_46 + x_10*x_46 + x_12*x_46 + x_13*x_46 + x_14*x_46 + x_15*x_46 + x_16*x_46 + x_17*x_46 + \n"
                       "        x_18*x_46 + x_19*x_46 + x_21*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_30*x_46 + x_31*x_46 + x_33*x_46 + x_43*x_46 + x_45*x_46 + x_1*x_47 + x_2*x_47 + x_4*x_47 + x_10*x_47 + \n"
                       "        x_15*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_27*x_47 + x_30*x_47 + x_32*x_47 + x_35*x_47 + x_36*x_47 + x_37*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + x_41*x_47 + x_42*x_47 + x_43*x_47 + \n"
                       "        x_45*x_47 + x_46*x_47 + x_7*x_48 + x_8*x_48 + x_10*x_48 + x_12*x_48 + x_16*x_48 + x_19*x_48 + x_20*x_48 + x_22*x_48 + x_23*x_48 + x_24*x_48 + x_25*x_48 + x_26*x_48 + x_28*x_48 + x_30*x_48 + \n"
                       "        x_31*x_48 + x_32*x_48 + x_33*x_48 + x_34*x_48 + x_36*x_48 + x_37*x_48 + x_39*x_48 + x_40*x_48 + x_44*x_48 + x_45*x_48 + x_5*x_49 + x_12*x_49 + x_13*x_49 + x_14*x_49 + x_18*x_49 + x_20*x_49 + \n"
                       "        x_23*x_49 + x_24*x_49 + x_26*x_49 + x_31*x_49 + x_32*x_49 + x_34*x_49 + x_36*x_49 + x_37*x_49 + x_38*x_49 + x_40*x_49 + x_42*x_49 + x_44*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_3 + x_6 + x_7 + x_8 \n"
                       "        + x_9 + x_13 + x_19 + x_21 + x_22 + x_27 + x_28 + x_30 + x_31 + x_33 + x_34 + x_35 + x_36 + x_37 + x_38 + x_40 + x_42 + x_44 + x_47,\n"
                       "    x_1*x_4 + x_2*x_4 + x_1*x_5 + x_2*x_5 + x_3*x_5 + x_5*x_7 + x_6*x_7 + x_1*x_8 + x_3*x_8 + x_4*x_8 + x_5*x_8 + x_7*x_8 + x_4*x_9 + x_5*x_9 + x_7*x_9 + x_8*x_9 + x_2*x_10 + x_3*x_10 + x_8*x_10 + \n"
                       "        x_5*x_11 + x_9*x_11 + x_5*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_10*x_13 + x_1*x_14 + x_3*x_14 + x_4*x_14 + x_5*x_14 + x_9*x_14 + x_10*x_14 + x_12*x_14 + x_2*x_15 + x_3*x_15 + x_5*x_15 + \n"
                       "        x_7*x_15 + x_8*x_15 + x_9*x_15 + x_10*x_15 + x_12*x_15 + x_13*x_15 + x_14*x_15 + x_1*x_16 + x_2*x_16 + x_4*x_16 + x_5*x_16 + x_9*x_16 + x_12*x_16 + x_13*x_16 + x_1*x_17 + x_3*x_17 + x_5*x_17 + \n"
                       "        x_6*x_17 + x_8*x_17 + x_9*x_17 + x_11*x_17 + x_12*x_17 + x_13*x_17 + x_1*x_18 + x_4*x_18 + x_5*x_18 + x_6*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_13*x_18 + x_14*x_18 + x_1*x_19 + \n"
                       "        x_2*x_19 + x_3*x_19 + x_5*x_19 + x_6*x_19 + x_7*x_19 + x_8*x_19 + x_12*x_19 + x_13*x_19 + x_16*x_19 + x_18*x_19 + x_1*x_20 + x_2*x_20 + x_4*x_20 + x_6*x_20 + x_7*x_20 + x_8*x_20 + x_10*x_20 + \n"
                       "        x_13*x_20 + x_14*x_20 + x_19*x_20 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_7*x_21 + x_11*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + x_15*x_21 + x_11*x_22 + x_13*x_22 + x_17*x_22 + x_18*x_22 + \n"
                       "        x_19*x_22 + x_21*x_22 + x_2*x_23 + x_3*x_23 + x_6*x_23 + x_7*x_23 + x_9*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_17*x_23 + x_18*x_23 + x_21*x_23 + x_2*x_24 + x_7*x_24 + x_9*x_24 + \n"
                       "        x_10*x_24 + x_11*x_24 + x_14*x_24 + x_23*x_24 + x_1*x_25 + x_2*x_25 + x_4*x_25 + x_7*x_25 + x_8*x_25 + x_10*x_25 + x_11*x_25 + x_13*x_25 + x_14*x_25 + x_17*x_25 + x_18*x_25 + x_20*x_25 + x_21*x_25\n"
                       "        + x_1*x_26 + x_3*x_26 + x_4*x_26 + x_8*x_26 + x_10*x_26 + x_11*x_26 + x_13*x_26 + x_17*x_26 + x_19*x_26 + x_20*x_26 + x_23*x_26 + x_25*x_26 + x_3*x_27 + x_4*x_27 + x_5*x_27 + x_7*x_27 + x_9*x_27 +\n"
                       "        x_11*x_27 + x_14*x_27 + x_19*x_27 + x_21*x_27 + x_24*x_27 + x_1*x_28 + x_4*x_28 + x_5*x_28 + x_7*x_28 + x_11*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_21*x_28 + x_22*x_28 + \n"
                       "        x_24*x_28 + x_26*x_28 + x_27*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_8*x_29 + x_9*x_29 + x_15*x_29 + x_17*x_29 + x_21*x_29 + x_1*x_30 + x_2*x_30 + x_3*x_30 + x_4*x_30 + x_8*x_30 + x_11*x_30 + \n"
                       "        x_15*x_30 + x_16*x_30 + x_17*x_30 + x_18*x_30 + x_19*x_30 + x_20*x_30 + x_21*x_30 + x_23*x_30 + x_24*x_30 + x_26*x_30 + x_1*x_31 + x_3*x_31 + x_10*x_31 + x_11*x_31 + x_13*x_31 + x_14*x_31 + \n"
                       "        x_15*x_31 + x_17*x_31 + x_18*x_31 + x_20*x_31 + x_24*x_31 + x_25*x_31 + x_27*x_31 + x_29*x_31 + x_30*x_31 + x_5*x_32 + x_6*x_32 + x_11*x_32 + x_14*x_32 + x_17*x_32 + x_19*x_32 + x_20*x_32 + \n"
                       "        x_22*x_32 + x_26*x_32 + x_30*x_32 + x_31*x_32 + x_3*x_33 + x_10*x_33 + x_13*x_33 + x_14*x_33 + x_18*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + \n"
                       "        x_31*x_33 + x_32*x_33 + x_1*x_34 + x_4*x_34 + x_5*x_34 + x_9*x_34 + x_10*x_34 + x_12*x_34 + x_14*x_34 + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_21*x_34 + x_24*x_34 + x_27*x_34 + \n"
                       "        x_30*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_2*x_35 + x_6*x_35 + x_8*x_35 + x_9*x_35 + x_11*x_35 + x_12*x_35 + x_14*x_35 + x_15*x_35 + x_17*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35 + \n"
                       "        x_23*x_35 + x_24*x_35 + x_25*x_35 + x_26*x_35 + x_27*x_35 + x_30*x_35 + x_31*x_35 + x_34*x_35 + x_1*x_36 + x_6*x_36 + x_8*x_36 + x_10*x_36 + x_14*x_36 + x_17*x_36 + x_18*x_36 + x_20*x_36 + \n"
                       "        x_21*x_36 + x_22*x_36 + x_23*x_36 + x_27*x_36 + x_28*x_36 + x_32*x_36 + x_34*x_36 + x_35*x_36 + x_3*x_37 + x_7*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_14*x_37 + x_19*x_37 + x_20*x_37 + \n"
                       "        x_21*x_37 + x_23*x_37 + x_27*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + x_4*x_38 + x_6*x_38 + x_9*x_38 + x_11*x_38 + x_13*x_38 + x_16*x_38 + x_17*x_38 + \n"
                       "        x_21*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + x_27*x_38 + x_29*x_38 + x_30*x_38 + x_36*x_38 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_7*x_39 + x_8*x_39 + x_9*x_39 + x_10*x_39 + x_20*x_39 +\n"
                       "        x_21*x_39 + x_23*x_39 + x_27*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_32*x_39 + x_33*x_39 + x_34*x_39 + x_5*x_40 + x_6*x_40 + x_8*x_40 + x_9*x_40 + x_10*x_40 + x_17*x_40 + \n"
                       "        x_18*x_40 + x_19*x_40 + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_29*x_40 + x_33*x_40 + x_34*x_40 + x_37*x_40 + x_1*x_41 + x_2*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + \n"
                       "        x_13*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_18*x_41 + x_19*x_41 + x_21*x_41 + x_25*x_41 + x_31*x_41 + x_32*x_41 + x_34*x_41 + x_36*x_41 + x_37*x_41 + x_2*x_42 + x_4*x_42 + x_5*x_42 + \n"
                       "        x_6*x_42 + x_7*x_42 + x_8*x_42 + x_15*x_42 + x_19*x_42 + x_21*x_42 + x_23*x_42 + x_25*x_42 + x_26*x_42 + x_37*x_42 + x_41*x_42 + x_1*x_43 + x_2*x_43 + x_4*x_43 + x_5*x_43 + x_7*x_43 + x_8*x_43 + \n"
                       "        x_10*x_43 + x_14*x_43 + x_16*x_43 + x_18*x_43 + x_21*x_43 + x_23*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 + x_28*x_43 + x_30*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_34*x_43 + x_35*x_43 + \n"
                       "        x_37*x_43 + x_38*x_43 + x_39*x_43 + x_41*x_43 + x_1*x_44 + x_5*x_44 + x_8*x_44 + x_13*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_21*x_44 + x_22*x_44 + x_24*x_44 + x_26*x_44 + x_28*x_44 + \n"
                       "        x_29*x_44 + x_32*x_44 + x_33*x_44 + x_34*x_44 + x_40*x_44 + x_43*x_44 + x_2*x_45 + x_3*x_45 + x_4*x_45 + x_8*x_45 + x_10*x_45 + x_11*x_45 + x_13*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + \n"
                       "        x_18*x_45 + x_19*x_45 + x_20*x_45 + x_21*x_45 + x_23*x_45 + x_24*x_45 + x_25*x_45 + x_29*x_45 + x_31*x_45 + x_33*x_45 + x_34*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + \n"
                       "        x_42*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_5*x_46 + x_7*x_46 + x_8*x_46 + x_10*x_46 + x_12*x_46 + x_14*x_46 + x_15*x_46 + x_16*x_46 + x_17*x_46 + x_18*x_46 + x_22*x_46 + x_26*x_46\n"
                       "        + x_27*x_46 + x_28*x_46 + x_32*x_46 + x_37*x_46 + x_38*x_46 + x_41*x_46 + x_43*x_46 + x_44*x_46 + x_1*x_47 + x_2*x_47 + x_3*x_47 + x_6*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_14*x_47 + \n"
                       "        x_18*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_24*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_36*x_47 + x_37*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + \n"
                       "        x_42*x_47 + x_44*x_47 + x_6*x_48 + x_8*x_48 + x_9*x_48 + x_11*x_48 + x_20*x_48 + x_21*x_48 + x_25*x_48 + x_27*x_48 + x_28*x_48 + x_29*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_40*x_48 + \n"
                       "        x_41*x_48 + x_42*x_48 + x_43*x_48 + x_44*x_48 + x_46*x_48 + x_2*x_49 + x_3*x_49 + x_4*x_49 + x_6*x_49 + x_7*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 + x_16*x_49 + x_17*x_49 + x_18*x_49 \n"
                       "        + x_19*x_49 + x_21*x_49 + x_22*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_28*x_49 + x_29*x_49 + x_31*x_49 + x_34*x_49 + x_36*x_49 + x_38*x_49 + x_39*x_49 + x_40*x_49 + x_45*x_49 + x_46*x_49 + \n"
                       "        x_48*x_49 + x_1 + x_2 + x_4 + x_6 + x_8 + x_9 + x_13 + x_16 + x_17 + x_18 + x_19 + x_20 + x_30 + x_31 + x_33 + x_34 + x_37 + x_40 + x_41 + x_42 + x_47 + x_48 + x_49,\n"
                       "    x_2*x_3 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_3*x_5 + x_2*x_6 + x_4*x_6 + x_2*x_7 + x_4*x_7 + x_5*x_7 + x_1*x_8 + x_3*x_8 + x_6*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_6*x_9 + x_7*x_9 + x_1*x_10 + x_3*x_10 \n"
                       "        + x_5*x_10 + x_9*x_10 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_6*x_12 + x_7*x_12 + x_9*x_12 + x_1*x_13 + x_2*x_13 + x_4*x_13 + \n"
                       "        x_5*x_13 + x_10*x_13 + x_11*x_13 + x_12*x_13 + x_1*x_14 + x_2*x_14 + x_3*x_14 + x_5*x_14 + x_12*x_14 + x_13*x_14 + x_2*x_15 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_9*x_15 + x_14*x_15 + x_1*x_16 + \n"
                       "        x_2*x_16 + x_4*x_16 + x_8*x_16 + x_9*x_16 + x_11*x_16 + x_1*x_17 + x_5*x_17 + x_6*x_17 + x_8*x_17 + x_13*x_17 + x_14*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_6*x_18 + x_7*x_18 + \n"
                       "        x_8*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_13*x_18 + x_14*x_18 + x_16*x_18 + x_17*x_18 + x_4*x_19 + x_5*x_19 + x_9*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_17*x_19 + x_18*x_19 + x_2*x_20 \n"
                       "        + x_3*x_20 + x_4*x_20 + x_5*x_20 + x_6*x_20 + x_8*x_20 + x_11*x_20 + x_12*x_20 + x_14*x_20 + x_17*x_20 + x_2*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_8*x_21 + x_9*x_21 + x_11*x_21 + x_13*x_21 + \n"
                       "        x_14*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_2*x_22 + x_3*x_22 + x_4*x_22 + x_7*x_22 + x_8*x_22 + x_10*x_22 + x_11*x_22 + x_13*x_22 + x_17*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_6*x_23 \n"
                       "        + x_7*x_23 + x_10*x_23 + x_12*x_23 + x_16*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23 + x_22*x_23 + x_5*x_24 + x_6*x_24 + x_8*x_24 + x_12*x_24 + x_17*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + \n"
                       "        x_22*x_24 + x_23*x_24 + x_2*x_25 + x_4*x_25 + x_5*x_25 + x_9*x_25 + x_12*x_25 + x_13*x_25 + x_14*x_25 + x_15*x_25 + x_17*x_25 + x_18*x_25 + x_19*x_25 + x_24*x_25 + x_1*x_26 + x_5*x_26 + x_9*x_26 +\n"
                       "        x_12*x_26 + x_14*x_26 + x_17*x_26 + x_21*x_26 + x_22*x_26 + x_23*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_5*x_27 + x_6*x_27 + x_8*x_27 + x_9*x_27 + x_11*x_27 + x_12*x_27 + x_14*x_27 \n"
                       "        + x_17*x_27 + x_19*x_27 + x_21*x_27 + x_23*x_27 + x_24*x_27 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 + x_14*x_28 + x_15*x_28 + x_19*x_28 + \n"
                       "        x_21*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_27*x_28 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_6*x_29 + x_8*x_29 + x_9*x_29 + x_12*x_29 + x_14*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_25*x_29 \n"
                       "        + x_26*x_29 + x_27*x_29 + x_1*x_30 + x_2*x_30 + x_4*x_30 + x_5*x_30 + x_6*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + x_13*x_30 + x_14*x_30 + x_16*x_30 + x_19*x_30 + x_20*x_30 + x_21*x_30 + \n"
                       "        x_22*x_30 + x_24*x_30 + x_25*x_30 + x_27*x_30 + x_29*x_30 + x_2*x_31 + x_7*x_31 + x_10*x_31 + x_11*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_21*x_31 + x_27*x_31 + x_29*x_31 + x_30*x_31 + \n"
                       "        x_4*x_32 + x_5*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_9*x_32 + x_11*x_32 + x_13*x_32 + x_14*x_32 + x_16*x_32 + x_17*x_32 + x_18*x_32 + x_20*x_32 + x_25*x_32 + x_27*x_32 + x_28*x_32 + x_2*x_33 +\n"
                       "        x_4*x_33 + x_5*x_33 + x_6*x_33 + x_7*x_33 + x_9*x_33 + x_11*x_33 + x_13*x_33 + x_15*x_33 + x_16*x_33 + x_19*x_33 + x_21*x_33 + x_22*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_28*x_33 + x_32*x_33\n"
                       "        + x_9*x_34 + x_12*x_34 + x_14*x_34 + x_16*x_34 + x_17*x_34 + x_20*x_34 + x_21*x_34 + x_22*x_34 + x_25*x_34 + x_26*x_34 + x_27*x_34 + x_29*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_1*x_35 + \n"
                       "        x_2*x_35 + x_3*x_35 + x_7*x_35 + x_8*x_35 + x_10*x_35 + x_11*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_17*x_35 + x_21*x_35 + x_22*x_35 + x_23*x_35 + x_27*x_35 + x_29*x_35 + x_30*x_35 + \n"
                       "        x_32*x_35 + x_33*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_7*x_36 + x_8*x_36 + x_12*x_36 + x_14*x_36 + x_18*x_36 + x_19*x_36 + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_29*x_36 + x_30*x_36 \n"
                       "        + x_31*x_36 + x_32*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_4*x_37 + x_7*x_37 + x_9*x_37 + x_12*x_37 + x_13*x_37 + x_17*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 + x_22*x_37 + \n"
                       "        x_23*x_37 + x_27*x_37 + x_28*x_37 + x_30*x_37 + x_31*x_37 + x_33*x_37 + x_34*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + x_2*x_38 + x_4*x_38 + x_5*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + \n"
                       "        x_12*x_38 + x_14*x_38 + x_15*x_38 + x_16*x_38 + x_18*x_38 + x_19*x_38 + x_20*x_38 + x_23*x_38 + x_25*x_38 + x_26*x_38 + x_28*x_38 + x_29*x_38 + x_31*x_38 + x_34*x_38 + x_37*x_38 + x_8*x_39 + \n"
                       "        x_14*x_39 + x_15*x_39 + x_18*x_39 + x_19*x_39 + x_21*x_39 + x_23*x_39 + x_24*x_39 + x_32*x_39 + x_33*x_39 + x_35*x_39 + x_38*x_39 + x_4*x_40 + x_5*x_40 + x_9*x_40 + x_11*x_40 + x_14*x_40 + \n"
                       "        x_19*x_40 + x_21*x_40 + x_22*x_40 + x_23*x_40 + x_25*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + x_33*x_40 + x_34*x_40 + x_37*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 \n"
                       "        + x_12*x_41 + x_13*x_41 + x_18*x_41 + x_20*x_41 + x_22*x_41 + x_23*x_41 + x_24*x_41 + x_27*x_41 + x_28*x_41 + x_29*x_41 + x_31*x_41 + x_32*x_41 + x_34*x_41 + x_36*x_41 + x_37*x_41 + x_38*x_41 + \n"
                       "        x_39*x_41 + x_4*x_42 + x_5*x_42 + x_8*x_42 + x_12*x_42 + x_13*x_42 + x_14*x_42 + x_18*x_42 + x_19*x_42 + x_21*x_42 + x_22*x_42 + x_24*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_33*x_42 + \n"
                       "        x_36*x_42 + x_38*x_42 + x_1*x_43 + x_2*x_43 + x_4*x_43 + x_5*x_43 + x_7*x_43 + x_8*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_18*x_43 + x_20*x_43 + x_21*x_43 + x_24*x_43 + x_26*x_43 \n"
                       "        + x_28*x_43 + x_32*x_43 + x_33*x_43 + x_34*x_43 + x_35*x_43 + x_38*x_43 + x_39*x_43 + x_40*x_43 + x_41*x_43 + x_2*x_44 + x_3*x_44 + x_5*x_44 + x_6*x_44 + x_7*x_44 + x_8*x_44 + x_10*x_44 + \n"
                       "        x_12*x_44 + x_13*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_19*x_44 + x_20*x_44 + x_21*x_44 + x_22*x_44 + x_24*x_44 + x_25*x_44 + x_26*x_44 + x_28*x_44 + x_30*x_44 + x_31*x_44 + x_37*x_44 + \n"
                       "        x_38*x_44 + x_39*x_44 + x_43*x_44 + x_9*x_45 + x_10*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_15*x_45 + x_16*x_45 + x_19*x_45 + x_22*x_45 + x_24*x_45 + x_27*x_45 + x_30*x_45 + x_32*x_45 + \n"
                       "        x_34*x_45 + x_35*x_45 + x_37*x_45 + x_38*x_45 + x_40*x_45 + x_1*x_46 + x_5*x_46 + x_6*x_46 + x_8*x_46 + x_9*x_46 + x_10*x_46 + x_12*x_46 + x_15*x_46 + x_17*x_46 + x_18*x_46 + x_19*x_46 + x_22*x_46\n"
                       "        + x_23*x_46 + x_24*x_46 + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_32*x_46 + x_35*x_46 + x_38*x_46 + x_39*x_46 + x_40*x_46 + x_41*x_46 + x_42*x_46 + x_44*x_46 + x_45*x_46 + x_3*x_47 + x_4*x_47 + \n"
                       "        x_5*x_47 + x_9*x_47 + x_12*x_47 + x_14*x_47 + x_15*x_47 + x_17*x_47 + x_19*x_47 + x_24*x_47 + x_25*x_47 + x_26*x_47 + x_27*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_32*x_47 + x_33*x_47 + \n"
                       "        x_35*x_47 + x_36*x_47 + x_38*x_47 + x_43*x_47 + x_46*x_47 + x_3*x_48 + x_4*x_48 + x_9*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_15*x_48 + x_16*x_48 + x_17*x_48 + x_21*x_48 + \n"
                       "        x_22*x_48 + x_25*x_48 + x_26*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_33*x_48 + x_35*x_48 + x_36*x_48 + x_42*x_48 + x_1*x_49 + x_3*x_49 + x_4*x_49 + x_5*x_49 + x_11*x_49 + x_13*x_49 + \n"
                       "        x_14*x_49 + x_15*x_49 + x_16*x_49 + x_17*x_49 + x_21*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_31*x_49 + x_34*x_49 + x_36*x_49 + x_37*x_49 + x_38*x_49 + x_39*x_49 + x_40*x_49 + x_43*x_49 + \n"
                       "        x_45*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_3 + x_7 + x_8 + x_9 + x_11 + x_12 + x_14 + x_15 + x_17 + x_20 + x_21 + x_22 + x_23 + x_25 + x_26 + x_27 + x_28 + x_29 + x_30 + x_33 + x_34 + x_35 + x_36\n"
                       "        + x_39 + x_41 + x_42 + x_43 + x_45 + x_46 + x_47 + x_49,\n"
                       "    x_1*x_2 + x_1*x_3 + x_2*x_3 + x_3*x_4 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_3*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_3*x_7 + x_4*x_7 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_3*x_8 + x_3*x_9 + x_4*x_9 + x_5*x_9 + \n"
                       "        x_6*x_9 + x_7*x_9 + x_8*x_9 + x_1*x_10 + x_2*x_10 + x_4*x_10 + x_5*x_10 + x_1*x_11 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_7*x_11 + x_10*x_11 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_7*x_12 + x_8*x_12 + \n"
                       "        x_11*x_12 + x_1*x_13 + x_3*x_13 + x_9*x_13 + x_10*x_13 + x_11*x_13 + x_3*x_14 + x_4*x_14 + x_12*x_14 + x_13*x_14 + x_2*x_15 + x_7*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_13*x_15 + x_14*x_15 + \n"
                       "        x_1*x_16 + x_5*x_16 + x_11*x_16 + x_1*x_17 + x_4*x_17 + x_6*x_17 + x_8*x_17 + x_10*x_17 + x_11*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + \n"
                       "        x_7*x_18 + x_8*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + x_17*x_18 + x_1*x_19 + x_3*x_19 + x_6*x_19 + x_9*x_19 + x_10*x_19 + x_11*x_19 + x_16*x_19 + x_3*x_20 + \n"
                       "        x_4*x_20 + x_6*x_20 + x_8*x_20 + x_9*x_20 + x_10*x_20 + x_12*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_18*x_20 + x_2*x_21 + x_5*x_21 + x_6*x_21 + x_12*x_21 + x_13*x_21 + x_15*x_21 + x_18*x_21 +\n"
                       "        x_19*x_21 + x_20*x_21 + x_1*x_22 + x_3*x_22 + x_6*x_22 + x_8*x_22 + x_9*x_22 + x_12*x_22 + x_13*x_22 + x_14*x_22 + x_18*x_22 + x_21*x_22 + x_2*x_23 + x_4*x_23 + x_7*x_23 + x_9*x_23 + x_13*x_23 + \n"
                       "        x_14*x_23 + x_18*x_23 + x_20*x_23 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_8*x_24 + x_9*x_24 + x_14*x_24 + x_16*x_24 + x_20*x_24 + x_21*x_24 + x_23*x_24 + x_1*x_25 + x_5*x_25 + \n"
                       "        x_10*x_25 + x_11*x_25 + x_12*x_25 + x_13*x_25 + x_14*x_25 + x_16*x_25 + x_17*x_25 + x_18*x_25 + x_20*x_25 + x_22*x_25 + x_23*x_25 + x_24*x_25 + x_1*x_26 + x_2*x_26 + x_4*x_26 + x_6*x_26 + x_8*x_26\n"
                       "        + x_11*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_19*x_26 + x_24*x_26 + x_1*x_27 + x_3*x_27 + x_5*x_27 + x_8*x_27 + x_9*x_27 + x_12*x_27 + x_13*x_27 + x_15*x_27 + x_17*x_27 + x_18*x_27 + \n"
                       "        x_20*x_27 + x_21*x_27 + x_23*x_27 + x_24*x_27 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_6*x_28 + x_7*x_28 + x_10*x_28 + x_11*x_28 + x_13*x_28 + x_15*x_28 + x_17*x_28 + x_20*x_28 + x_21*x_28 + x_22*x_28\n"
                       "        + x_24*x_28 + x_27*x_28 + x_1*x_29 + x_3*x_29 + x_4*x_29 + x_7*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_13*x_29 + x_15*x_29 + x_16*x_29 + x_18*x_29 + x_19*x_29 + x_21*x_29 + x_22*x_29 + \n"
                       "        x_26*x_29 + x_27*x_29 + x_1*x_30 + x_2*x_30 + x_6*x_30 + x_7*x_30 + x_9*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + x_13*x_30 + x_15*x_30 + x_16*x_30 + x_18*x_30 + x_19*x_30 + x_20*x_30 + x_21*x_30\n"
                       "        + x_22*x_30 + x_24*x_30 + x_25*x_30 + x_27*x_30 + x_28*x_30 + x_3*x_31 + x_5*x_31 + x_6*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 + x_17*x_31 + x_18*x_31 + x_19*x_31 + x_20*x_31 + \n"
                       "        x_22*x_31 + x_28*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32 + x_5*x_32 + x_6*x_32 + x_7*x_32 + x_10*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_16*x_32 + x_18*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32\n"
                       "        + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_29*x_32 + x_31*x_32 + x_3*x_33 + x_5*x_33 + x_6*x_33 + x_9*x_33 + x_10*x_33 + x_15*x_33 + x_19*x_33 + x_20*x_33 + x_22*x_33 + x_26*x_33 + x_29*x_33 + \n"
                       "        x_31*x_33 + x_1*x_34 + x_2*x_34 + x_4*x_34 + x_5*x_34 + x_8*x_34 + x_9*x_34 + x_10*x_34 + x_12*x_34 + x_13*x_34 + x_17*x_34 + x_18*x_34 + x_21*x_34 + x_23*x_34 + x_24*x_34 + x_25*x_34 + x_27*x_34 \n"
                       "        + x_28*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_1*x_35 + x_4*x_35 + x_7*x_35 + x_8*x_35 + x_9*x_35 + x_11*x_35 + x_12*x_35 + x_13*x_35 + x_19*x_35 + x_20*x_35 + x_23*x_35 + x_24*x_35 + \n"
                       "        x_25*x_35 + x_27*x_35 + x_28*x_35 + x_29*x_35 + x_31*x_35 + x_33*x_35 + x_2*x_36 + x_3*x_36 + x_6*x_36 + x_8*x_36 + x_13*x_36 + x_14*x_36 + x_17*x_36 + x_18*x_36 + x_21*x_36 + x_22*x_36 + \n"
                       "        x_27*x_36 + x_28*x_36 + x_29*x_36 + x_31*x_36 + x_3*x_37 + x_5*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 + x_13*x_37 + x_17*x_37 + x_20*x_37 + x_21*x_37 \n"
                       "        + x_22*x_37 + x_23*x_37 + x_33*x_37 + x_36*x_37 + x_2*x_38 + x_4*x_38 + x_6*x_38 + x_13*x_38 + x_14*x_38 + x_15*x_38 + x_19*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + x_34*x_38 + x_35*x_38 + \n"
                       "        x_36*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + x_3*x_39 + x_5*x_39 + x_6*x_39 + x_11*x_39 + x_13*x_39 + x_19*x_39 + x_21*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + x_26*x_39 + x_27*x_39 + x_28*x_39\n"
                       "        + x_30*x_39 + x_31*x_39 + x_33*x_39 + x_37*x_39 + x_1*x_40 + x_3*x_40 + x_4*x_40 + x_9*x_40 + x_14*x_40 + x_18*x_40 + x_20*x_40 + x_21*x_40 + x_22*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + \n"
                       "        x_26*x_40 + x_31*x_40 + x_32*x_40 + x_35*x_40 + x_38*x_40 + x_1*x_41 + x_2*x_41 + x_4*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_17*x_41 +\n"
                       "        x_18*x_41 + x_19*x_41 + x_20*x_41 + x_22*x_41 + x_23*x_41 + x_24*x_41 + x_28*x_41 + x_30*x_41 + x_34*x_41 + x_35*x_41 + x_39*x_41 + x_40*x_41 + x_2*x_42 + x_3*x_42 + x_9*x_42 + x_10*x_42 + \n"
                       "        x_11*x_42 + x_19*x_42 + x_21*x_42 + x_22*x_42 + x_23*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_32*x_42 + x_35*x_42 + x_36*x_42 + x_39*x_42 + x_41*x_42 + x_2*x_43 + x_5*x_43 + x_7*x_43 + \n"
                       "        x_9*x_43 + x_10*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_17*x_43 + x_18*x_43 + x_20*x_43 + x_23*x_43 + x_24*x_43 + x_27*x_43 + x_30*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + \n"
                       "        x_35*x_43 + x_37*x_43 + x_38*x_43 + x_40*x_43 + x_41*x_43 + x_42*x_43 + x_1*x_44 + x_3*x_44 + x_4*x_44 + x_5*x_44 + x_7*x_44 + x_9*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_14*x_44 + x_17*x_44 \n"
                       "        + x_19*x_44 + x_22*x_44 + x_23*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_33*x_44 + x_34*x_44 + x_36*x_44 + x_37*x_44 + x_40*x_44 + x_41*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + \n"
                       "        x_5*x_45 + x_9*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_14*x_45 + x_16*x_45 + x_19*x_45 + x_20*x_45 + x_23*x_45 + x_24*x_45 + x_28*x_45 + x_29*x_45 + x_30*x_45 + x_31*x_45 + x_32*x_45 + \n"
                       "        x_33*x_45 + x_34*x_45 + x_41*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_7*x_46 + x_8*x_46 + x_13*x_46 + x_16*x_46 + x_19*x_46 + x_22*x_46 + x_23*x_46 + x_25*x_46 + x_26*x_46 + x_28*x_46 + x_34*x_46\n"
                       "        + x_35*x_46 + x_39*x_46 + x_40*x_46 + x_41*x_46 + x_43*x_46 + x_2*x_47 + x_3*x_47 + x_6*x_47 + x_7*x_47 + x_8*x_47 + x_14*x_47 + x_15*x_47 + x_17*x_47 + x_18*x_47 + x_20*x_47 + x_21*x_47 + \n"
                       "        x_23*x_47 + x_26*x_47 + x_27*x_47 + x_28*x_47 + x_29*x_47 + x_31*x_47 + x_36*x_47 + x_43*x_47 + x_3*x_48 + x_4*x_48 + x_6*x_48 + x_8*x_48 + x_9*x_48 + x_11*x_48 + x_13*x_48 + x_18*x_48 + x_20*x_48\n"
                       "        + x_22*x_48 + x_24*x_48 + x_25*x_48 + x_29*x_48 + x_33*x_48 + x_34*x_48 + x_35*x_48 + x_37*x_48 + x_39*x_48 + x_41*x_48 + x_44*x_48 + x_1*x_49 + x_3*x_49 + x_7*x_49 + x_10*x_49 + x_11*x_49 + \n"
                       "        x_13*x_49 + x_16*x_49 + x_18*x_49 + x_19*x_49 + x_20*x_49 + x_22*x_49 + x_23*x_49 + x_26*x_49 + x_27*x_49 + x_30*x_49 + x_31*x_49 + x_32*x_49 + x_33*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + \n"
                       "        x_37*x_49 + x_39*x_49 + x_41*x_49 + x_42*x_49 + x_43*x_49 + x_45*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_3 + x_4 + x_5 + x_8 + x_10 + x_12 + x_15 + x_17 + x_18 + x_19 + x_20 + x_21 + x_26 + x_27 + \n"
                       "        x_30 + x_33 + x_34 + x_36 + x_37 + x_42 + x_43 + x_44 + x_48 + x_49,\n"
                       "    x_1*x_3 + x_2*x_3 + x_3*x_4 + x_1*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + x_4*x_6 + x_1*x_7 + x_2*x_7 + x_4*x_7 + x_5*x_7 + x_6*x_7 + x_3*x_8 + x_4*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_6*x_9 + x_8*x_9 + \n"
                       "        x_2*x_10 + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_8*x_11 + x_10*x_11 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_10*x_12 + x_2*x_13 + x_6*x_13 + \n"
                       "        x_7*x_13 + x_9*x_13 + x_1*x_14 + x_4*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14 + x_12*x_14 + x_13*x_14 + x_1*x_15 + x_3*x_15 + x_4*x_15 + x_6*x_15 + x_8*x_15 + x_9*x_15 + x_11*x_15 + x_12*x_15 + \n"
                       "        x_13*x_15 + x_1*x_16 + x_2*x_16 + x_3*x_16 + x_6*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_15*x_16 + x_2*x_17 + x_8*x_17 + x_10*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + \n"
                       "        x_4*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_13*x_18 + x_14*x_18 + x_4*x_19 + x_5*x_19 + x_6*x_19 + x_8*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_14*x_19 + x_15*x_19 + x_17*x_19 + x_2*x_20 + \n"
                       "        x_5*x_20 + x_7*x_20 + x_8*x_20 + x_10*x_20 + x_11*x_20 + x_12*x_20 + x_17*x_20 + x_19*x_20 + x_3*x_21 + x_5*x_21 + x_8*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + x_15*x_21 + x_17*x_21 + x_18*x_21 \n"
                       "        + x_20*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_4*x_22 + x_6*x_22 + x_7*x_22 + x_12*x_22 + x_13*x_22 + x_14*x_22 + x_15*x_22 + x_18*x_22 + x_19*x_22 + x_21*x_22 + x_2*x_23 + x_3*x_23 + x_4*x_23 +\n"
                       "        x_9*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23 + x_21*x_23 + x_22*x_23 + x_2*x_24 + x_7*x_24 + x_10*x_24 + x_11*x_24 + x_12*x_24 + \n"
                       "        x_13*x_24 + x_16*x_24 + x_17*x_24 + x_20*x_24 + x_1*x_25 + x_3*x_25 + x_4*x_25 + x_6*x_25 + x_8*x_25 + x_10*x_25 + x_13*x_25 + x_14*x_25 + x_16*x_25 + x_17*x_25 + x_18*x_25 + x_19*x_25 + x_21*x_25\n"
                       "        + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_4*x_26 + x_5*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_10*x_26 + x_11*x_26 + x_12*x_26 + x_13*x_26 + x_15*x_26 + x_17*x_26 + x_20*x_26 + x_25*x_26 + x_1*x_27 +\n"
                       "        x_2*x_27 + x_6*x_27 + x_7*x_27 + x_10*x_27 + x_11*x_27 + x_12*x_27 + x_13*x_27 + x_15*x_27 + x_17*x_27 + x_18*x_27 + x_20*x_27 + x_22*x_27 + x_24*x_27 + x_26*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 \n"
                       "        + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_8*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + x_15*x_28 + x_18*x_28 + x_19*x_28 + x_20*x_28 + x_24*x_28 + x_26*x_28 + \n"
                       "        x_2*x_29 + x_3*x_29 + x_4*x_29 + x_7*x_29 + x_11*x_29 + x_12*x_29 + x_13*x_29 + x_14*x_29 + x_15*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 + x_20*x_29 + x_26*x_29 + x_1*x_30 + x_7*x_30 + x_9*x_30 +\n"
                       "        x_10*x_30 + x_12*x_30 + x_15*x_30 + x_16*x_30 + x_17*x_30 + x_18*x_30 + x_24*x_30 + x_27*x_30 + x_28*x_30 + x_29*x_30 + x_1*x_31 + x_3*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_9*x_31 + x_11*x_31 \n"
                       "        + x_12*x_31 + x_13*x_31 + x_14*x_31 + x_17*x_31 + x_18*x_31 + x_22*x_31 + x_23*x_31 + x_29*x_31 + x_30*x_31 + x_2*x_32 + x_3*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + \n"
                       "        x_12*x_32 + x_13*x_32 + x_14*x_32 + x_17*x_32 + x_20*x_32 + x_21*x_32 + x_23*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_29*x_32 + x_30*x_32 + x_1*x_33 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_6*x_33\n"
                       "        + x_7*x_33 + x_9*x_33 + x_15*x_33 + x_17*x_33 + x_20*x_33 + x_25*x_33 + x_29*x_33 + x_1*x_34 + x_2*x_34 + x_3*x_34 + x_5*x_34 + x_8*x_34 + x_9*x_34 + x_11*x_34 + x_13*x_34 + x_14*x_34 + x_16*x_34 \n"
                       "        + x_17*x_34 + x_21*x_34 + x_24*x_34 + x_26*x_34 + x_27*x_34 + x_29*x_34 + x_31*x_34 + x_33*x_34 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_6*x_35 + x_7*x_35 + x_10*x_35 + x_12*x_35 + x_15*x_35 + \n"
                       "        x_16*x_35 + x_17*x_35 + x_20*x_35 + x_21*x_35 + x_23*x_35 + x_24*x_35 + x_25*x_35 + x_27*x_35 + x_28*x_35 + x_29*x_35 + x_31*x_35 + x_1*x_36 + x_2*x_36 + x_5*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 \n"
                       "        + x_12*x_36 + x_15*x_36 + x_17*x_36 + x_19*x_36 + x_20*x_36 + x_21*x_36 + x_22*x_36 + x_24*x_36 + x_30*x_36 + x_31*x_36 + x_32*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + \n"
                       "        x_7*x_37 + x_10*x_37 + x_11*x_37 + x_13*x_37 + x_16*x_37 + x_17*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 + x_22*x_37 + x_25*x_37 + x_26*x_37 + x_28*x_37 + x_30*x_37 + x_33*x_37 + x_34*x_37 + \n"
                       "        x_1*x_38 + x_4*x_38 + x_6*x_38 + x_8*x_38 + x_12*x_38 + x_13*x_38 + x_14*x_38 + x_15*x_38 + x_17*x_38 + x_20*x_38 + x_22*x_38 + x_23*x_38 + x_25*x_38 + x_26*x_38 + x_27*x_38 + x_28*x_38 + \n"
                       "        x_31*x_38 + x_36*x_38 + x_37*x_38 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_7*x_39 + x_8*x_39 + x_9*x_39 + x_11*x_39 + x_12*x_39 + x_13*x_39 + x_15*x_39 + x_16*x_39 + x_17*x_39 + x_18*x_39 +\n"
                       "        x_19*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_26*x_39 + x_27*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_33*x_39 + x_34*x_39 + x_35*x_39 + x_2*x_40 + x_3*x_40 + x_5*x_40 + x_6*x_40 + x_8*x_40\n"
                       "        + x_9*x_40 + x_10*x_40 + x_12*x_40 + x_13*x_40 + x_16*x_40 + x_18*x_40 + x_19*x_40 + x_21*x_40 + x_22*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + \n"
                       "        x_31*x_40 + x_33*x_40 + x_36*x_40 + x_37*x_40 + x_2*x_41 + x_3*x_41 + x_5*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_11*x_41 + x_14*x_41 + x_15*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41\n"
                       "        + x_24*x_41 + x_25*x_41 + x_26*x_41 + x_27*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_35*x_41 + x_38*x_41 + x_39*x_41 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_8*x_42 + x_9*x_42 + x_10*x_42 + \n"
                       "        x_12*x_42 + x_13*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_20*x_42 + x_24*x_42 + x_26*x_42 + x_27*x_42 + x_29*x_42 + x_31*x_42 + x_32*x_42 + x_34*x_42 + x_35*x_42 + x_40*x_42 + x_41*x_42 + \n"
                       "        x_1*x_43 + x_2*x_43 + x_8*x_43 + x_9*x_43 + x_11*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_18*x_43 + x_20*x_43 + x_23*x_43 + x_24*x_43 + x_25*x_43 + x_27*x_43 + \n"
                       "        x_28*x_43 + x_30*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_36*x_43 + x_41*x_43 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_5*x_44 + x_8*x_44 + x_11*x_44 + x_14*x_44 + x_15*x_44 + x_17*x_44 + x_18*x_44\n"
                       "        + x_19*x_44 + x_20*x_44 + x_22*x_44 + x_23*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_29*x_44 + x_30*x_44 + x_33*x_44 + x_37*x_44 + x_38*x_44 + x_40*x_44 + x_42*x_44 + x_43*x_44 + x_3*x_45 + \n"
                       "        x_4*x_45 + x_6*x_45 + x_7*x_45 + x_12*x_45 + x_15*x_45 + x_16*x_45 + x_25*x_45 + x_27*x_45 + x_28*x_45 + x_29*x_45 + x_31*x_45 + x_34*x_45 + x_35*x_45 + x_36*x_45 + x_37*x_45 + x_38*x_45 + \n"
                       "        x_39*x_45 + x_40*x_45 + x_41*x_45 + x_42*x_45 + x_43*x_45 + x_44*x_45 + x_4*x_46 + x_6*x_46 + x_7*x_46 + x_8*x_46 + x_9*x_46 + x_11*x_46 + x_13*x_46 + x_14*x_46 + x_21*x_46 + x_24*x_46 + x_28*x_46\n"
                       "        + x_33*x_46 + x_35*x_46 + x_39*x_46 + x_42*x_46 + x_45*x_46 + x_3*x_47 + x_4*x_47 + x_6*x_47 + x_7*x_47 + x_11*x_47 + x_15*x_47 + x_17*x_47 + x_22*x_47 + x_23*x_47 + x_24*x_47 + x_27*x_47 + \n"
                       "        x_29*x_47 + x_31*x_47 + x_33*x_47 + x_35*x_47 + x_37*x_47 + x_40*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_46*x_47 + x_2*x_48 + x_4*x_48 + x_7*x_48 + x_10*x_48 + x_14*x_48 + x_15*x_48 + \n"
                       "        x_16*x_48 + x_17*x_48 + x_18*x_48 + x_21*x_48 + x_27*x_48 + x_28*x_48 + x_29*x_48 + x_31*x_48 + x_35*x_48 + x_37*x_48 + x_38*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_43*x_48 + x_45*x_48 + \n"
                       "        x_47*x_48 + x_2*x_49 + x_3*x_49 + x_4*x_49 + x_6*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_17*x_49 + x_20*x_49 + x_21*x_49 + x_22*x_49 \n"
                       "        + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_31*x_49 + x_32*x_49 + x_33*x_49 + x_35*x_49 + x_37*x_49 + x_39*x_49 + x_41*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_3 + \n"
                       "        x_5 + x_8 + x_10 + x_12 + x_14 + x_15 + x_18 + x_19 + x_21 + x_22 + x_26 + x_27 + x_35 + x_36 + x_37 + x_41 + x_44 + x_48 + x_49,\n"
                       "    x_1*x_2 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_3*x_5 + x_1*x_7 + x_2*x_7 + x_4*x_7 + x_1*x_8 + x_2*x_8 + x_3*x_8 + x_6*x_8 + x_1*x_9 + x_5*x_9 + x_7*x_9 + x_8*x_9 + x_3*x_10 + x_5*x_10 + x_7*x_10 + \n"
                       "        x_8*x_10 + x_1*x_11 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_8*x_11 + x_9*x_11 + x_10*x_11 + x_1*x_12 + x_2*x_12 + x_4*x_12 + x_8*x_12 + x_9*x_12 + x_1*x_13 + x_3*x_13 + x_8*x_13 + x_4*x_14\n"
                       "        + x_5*x_14 + x_8*x_14 + x_12*x_14 + x_13*x_14 + x_1*x_15 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_6*x_15 + x_7*x_15 + x_8*x_15 + x_10*x_15 + x_13*x_15 + x_14*x_15 + x_4*x_16 + x_6*x_16 + x_7*x_16 + \n"
                       "        x_8*x_16 + x_13*x_16 + x_2*x_17 + x_3*x_17 + x_6*x_17 + x_7*x_17 + x_9*x_17 + x_10*x_17 + x_11*x_17 + x_14*x_17 + x_15*x_17 + x_2*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_14*x_18 + \n"
                       "        x_15*x_18 + x_17*x_18 + x_1*x_19 + x_2*x_19 + x_3*x_19 + x_9*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_14*x_19 + x_17*x_19 + x_18*x_19 + x_5*x_20 + x_7*x_20 + x_9*x_20 + x_11*x_20 + x_15*x_20 +\n"
                       "        x_16*x_20 + x_17*x_20 + x_1*x_21 + x_2*x_21 + x_3*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_9*x_21 + x_10*x_21 + x_11*x_21 + x_13*x_21 + x_15*x_21 + x_17*x_21 + x_18*x_21 + x_1*x_22 + x_2*x_22 + \n"
                       "        x_4*x_22 + x_14*x_22 + x_15*x_22 + x_19*x_22 + x_21*x_22 + x_1*x_23 + x_5*x_23 + x_6*x_23 + x_8*x_23 + x_11*x_23 + x_14*x_23 + x_15*x_23 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_7*x_24 + x_8*x_24 + \n"
                       "        x_9*x_24 + x_10*x_24 + x_16*x_24 + x_17*x_24 + x_21*x_24 + x_23*x_24 + x_2*x_25 + x_3*x_25 + x_5*x_25 + x_6*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 + x_14*x_25 + x_16*x_25 + x_18*x_25 \n"
                       "        + x_19*x_25 + x_21*x_25 + x_24*x_25 + x_3*x_26 + x_4*x_26 + x_6*x_26 + x_7*x_26 + x_10*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_17*x_26 + x_19*x_26 + x_21*x_26 + x_22*x_26 + x_23*x_26 + \n"
                       "        x_24*x_26 + x_25*x_26 + x_1*x_27 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_8*x_27 + x_10*x_27 + x_11*x_27 + x_13*x_27 + x_14*x_27 + x_15*x_27 + x_17*x_27 + x_18*x_27 + x_22*x_27 + x_23*x_27 + x_24*x_27\n"
                       "        + x_1*x_28 + x_5*x_28 + x_8*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_19*x_28 + x_20*x_28 + x_22*x_28 + x_23*x_28 + x_26*x_28 + x_3*x_29 + x_7*x_29 + x_11*x_29 + x_12*x_29 + x_14*x_29 + \n"
                       "        x_15*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_24*x_29 + x_28*x_29 + x_1*x_30 + x_4*x_30 + x_5*x_30 + x_10*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_16*x_30 + \n"
                       "        x_18*x_30 + x_19*x_30 + x_20*x_30 + x_21*x_30 + x_22*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + x_29*x_30 + x_1*x_31 + x_4*x_31 + x_5*x_31 + x_8*x_31 + x_11*x_31 + x_12*x_31 + x_13*x_31 + \n"
                       "        x_14*x_31 + x_15*x_31 + x_17*x_31 + x_19*x_31 + x_20*x_31 + x_22*x_31 + x_23*x_31 + x_24*x_31 + x_26*x_31 + x_30*x_31 + x_1*x_32 + x_8*x_32 + x_9*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + \n"
                       "        x_14*x_32 + x_15*x_32 + x_17*x_32 + x_18*x_32 + x_22*x_32 + x_25*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_3*x_33 + x_8*x_33 + x_9*x_33 + x_11*x_33 + x_13*x_33 + \n"
                       "        x_16*x_33 + x_17*x_33 + x_19*x_33 + x_23*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_29*x_33 + x_30*x_33 + x_32*x_33 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_7*x_34 + x_10*x_34 + x_12*x_34 + \n"
                       "        x_13*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_21*x_34 + x_24*x_34 + x_26*x_34 + x_27*x_34 + x_28*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_3*x_35 + x_7*x_35 + x_8*x_35 + \n"
                       "        x_10*x_35 + x_12*x_35 + x_13*x_35 + x_16*x_35 + x_17*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35 + x_22*x_35 + x_24*x_35 + x_26*x_35 + x_28*x_35 + x_29*x_35 + x_32*x_35 + x_33*x_35 + x_1*x_36 + \n"
                       "        x_3*x_36 + x_4*x_36 + x_10*x_36 + x_13*x_36 + x_14*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + x_20*x_36 + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 + x_27*x_36 + x_28*x_36 + x_30*x_36 + \n"
                       "        x_32*x_36 + x_33*x_36 + x_34*x_36 + x_35*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_7*x_37 + x_9*x_37 + x_11*x_37 + x_13*x_37 + x_16*x_37 + x_17*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 \n"
                       "        + x_22*x_37 + x_25*x_37 + x_26*x_37 + x_27*x_37 + x_28*x_37 + x_30*x_37 + x_31*x_37 + x_32*x_37 + x_36*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 \n"
                       "        + x_14*x_38 + x_15*x_38 + x_20*x_38 + x_22*x_38 + x_24*x_38 + x_25*x_38 + x_26*x_38 + x_27*x_38 + x_28*x_38 + x_31*x_38 + x_33*x_38 + x_34*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + x_3*x_39 + \n"
                       "        x_4*x_39 + x_7*x_39 + x_8*x_39 + x_11*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_24*x_39 + x_26*x_39 + x_27*x_39 + x_32*x_39 + x_35*x_39 + x_36*x_39 + x_37*x_39 + x_38*x_39 + x_1*x_40 + x_5*x_40\n"
                       "        + x_7*x_40 + x_8*x_40 + x_10*x_40 + x_11*x_40 + x_12*x_40 + x_13*x_40 + x_14*x_40 + x_18*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + x_23*x_40 + x_25*x_40 + x_26*x_40 + x_29*x_40 + x_30*x_40 + \n"
                       "        x_33*x_40 + x_36*x_40 + x_38*x_40 + x_2*x_41 + x_3*x_41 + x_5*x_41 + x_6*x_41 + x_8*x_41 + x_10*x_41 + x_11*x_41 + x_13*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_19*x_41 + x_24*x_41 + x_26*x_41\n"
                       "        + x_27*x_41 + x_30*x_41 + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_38*x_41 + x_39*x_41 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_13*x_42 + x_14*x_42 + x_17*x_42 + x_19*x_42 + x_22*x_42 + x_24*x_42 + \n"
                       "        x_28*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_35*x_42 + x_37*x_42 + x_2*x_43 + x_3*x_43 + x_4*x_43 + x_5*x_43 + x_6*x_43 + x_7*x_43 + x_8*x_43 + x_10*x_43 + x_11*x_43 + x_14*x_43 +\n"
                       "        x_15*x_43 + x_16*x_43 + x_19*x_43 + x_20*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + x_26*x_43 + x_29*x_43 + x_30*x_43 + x_31*x_43 + x_33*x_43 + x_34*x_43 + x_37*x_43 + x_38*x_43 + x_40*x_43 + \n"
                       "        x_41*x_43 + x_42*x_43 + x_7*x_44 + x_9*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_23*x_44 + x_24*x_44 + x_26*x_44 + x_27*x_44 + x_30*x_44 + x_32*x_44 + \n"
                       "        x_33*x_44 + x_34*x_44 + x_36*x_44 + x_37*x_44 + x_42*x_44 + x_43*x_44 + x_2*x_45 + x_4*x_45 + x_9*x_45 + x_10*x_45 + x_11*x_45 + x_14*x_45 + x_16*x_45 + x_18*x_45 + x_19*x_45 + x_22*x_45 + \n"
                       "        x_24*x_45 + x_25*x_45 + x_26*x_45 + x_27*x_45 + x_29*x_45 + x_32*x_45 + x_34*x_45 + x_37*x_45 + x_40*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_3*x_46 + x_7*x_46 + x_8*x_46\n"
                       "        + x_15*x_46 + x_16*x_46 + x_17*x_46 + x_18*x_46 + x_19*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_27*x_46 + x_33*x_46 + x_35*x_46 + x_39*x_46 + x_42*x_46 + x_44*x_46 + x_1*x_47 + x_2*x_47 + \n"
                       "        x_4*x_47 + x_5*x_47 + x_7*x_47 + x_8*x_47 + x_9*x_47 + x_12*x_47 + x_13*x_47 + x_16*x_47 + x_18*x_47 + x_19*x_47 + x_20*x_47 + x_22*x_47 + x_23*x_47 + x_25*x_47 + x_27*x_47 + x_28*x_47 + x_30*x_47\n"
                       "        + x_31*x_47 + x_33*x_47 + x_37*x_47 + x_38*x_47 + x_41*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_4*x_48 + x_8*x_48 + x_10*x_48 + x_11*x_48 + x_15*x_48 + x_16*x_48 + x_17*x_48 + x_19*x_48 + \n"
                       "        x_22*x_48 + x_24*x_48 + x_27*x_48 + x_28*x_48 + x_29*x_48 + x_30*x_48 + x_31*x_48 + x_32*x_48 + x_34*x_48 + x_35*x_48 + x_37*x_48 + x_38*x_48 + x_43*x_48 + x_45*x_48 + x_47*x_48 + x_7*x_49 + \n"
                       "        x_8*x_49 + x_10*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_17*x_49 + x_18*x_49 + x_19*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_29*x_49 + x_35*x_49 + x_37*x_49 + x_38*x_49 + x_39*x_49 + \n"
                       "        x_43*x_49 + x_1 + x_5 + x_8 + x_12 + x_14 + x_16 + x_18 + x_20 + x_22 + x_24 + x_27 + x_31 + x_33 + x_34 + x_35 + x_37 + x_38 + x_41 + x_45 + x_46 + x_47 + x_49,\n"
                       "    x_1*x_3 + x_2*x_3 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_5 + x_5*x_6 + x_2*x_7 + x_3*x_7 + x_6*x_7 + x_1*x_8 + x_2*x_8 + x_4*x_8 + x_6*x_8 + x_1*x_9 + x_2*x_9 + x_3*x_9 + x_5*x_9 + x_7*x_9 + x_2*x_10 +\n"
                       "        x_3*x_10 + x_8*x_10 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_6*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_1*x_12 + x_2*x_12 + x_5*x_12 + x_7*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_3*x_13 + \n"
                       "        x_4*x_13 + x_6*x_13 + x_8*x_13 + x_2*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14 + x_8*x_14 + x_9*x_14 + x_12*x_14 + x_13*x_14 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_12*x_15 + \n"
                       "        x_13*x_15 + x_14*x_15 + x_3*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_7*x_17 + x_10*x_17 + x_11*x_17 + \n"
                       "        x_12*x_17 + x_14*x_17 + x_15*x_17 + x_4*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_9*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + x_17*x_18 + x_2*x_19 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_10*x_19 + \n"
                       "        x_11*x_19 + x_13*x_19 + x_15*x_19 + x_17*x_19 + x_18*x_19 + x_6*x_20 + x_9*x_20 + x_10*x_20 + x_11*x_20 + x_12*x_20 + x_13*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20 + x_1*x_21 + \n"
                       "        x_4*x_21 + x_6*x_21 + x_11*x_21 + x_12*x_21 + x_14*x_21 + x_15*x_21 + x_16*x_21 + x_19*x_21 + x_20*x_21 + x_2*x_22 + x_4*x_22 + x_5*x_22 + x_6*x_22 + x_7*x_22 + x_15*x_22 + x_16*x_22 + x_18*x_22 +\n"
                       "        x_19*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_2*x_23 + x_5*x_23 + x_7*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_14*x_23 + x_15*x_23 + x_16*x_23 + x_18*x_23 + x_19*x_23 + x_1*x_24\n"
                       "        + x_2*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_14*x_24 + x_15*x_24 + x_16*x_24 + x_17*x_24 + x_21*x_24 + x_23*x_24 + x_2*x_25 + x_3*x_25 + x_4*x_25 + x_7*x_25 +\n"
                       "        x_8*x_25 + x_11*x_25 + x_15*x_25 + x_22*x_25 + x_3*x_26 + x_4*x_26 + x_5*x_26 + x_6*x_26 + x_9*x_26 + x_11*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_16*x_26 + x_19*x_26 + x_20*x_26 \n"
                       "        + x_21*x_26 + x_24*x_26 + x_9*x_27 + x_11*x_27 + x_12*x_27 + x_16*x_27 + x_18*x_27 + x_21*x_27 + x_23*x_27 + x_24*x_27 + x_25*x_27 + x_2*x_28 + x_4*x_28 + x_7*x_28 + x_8*x_28 + x_9*x_28 + \n"
                       "        x_10*x_28 + x_11*x_28 + x_12*x_28 + x_13*x_28 + x_17*x_28 + x_18*x_28 + x_19*x_28 + x_21*x_28 + x_22*x_28 + x_23*x_28 + x_25*x_28 + x_26*x_28 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_6*x_29\n"
                       "        + x_7*x_29 + x_9*x_29 + x_12*x_29 + x_13*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_25*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 + x_3*x_30 + x_5*x_30 + x_7*x_30\n"
                       "        + x_8*x_30 + x_11*x_30 + x_12*x_30 + x_15*x_30 + x_16*x_30 + x_20*x_30 + x_21*x_30 + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_28*x_30 + x_2*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 + x_12*x_31 + \n"
                       "        x_13*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_21*x_31 + x_22*x_31 + x_25*x_31 + x_27*x_31 + x_29*x_31 + x_30*x_31 + x_2*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + \n"
                       "        x_16*x_32 + x_18*x_32 + x_20*x_32 + x_23*x_32 + x_24*x_32 + x_25*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_2*x_33 + x_3*x_33 + x_5*x_33 + x_6*x_33 + x_7*x_33 \n"
                       "        + x_11*x_33 + x_14*x_33 + x_19*x_33 + x_22*x_33 + x_25*x_33 + x_26*x_33 + x_28*x_33 + x_29*x_33 + x_30*x_33 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_9*x_34 + x_10*x_34 + x_14*x_34 + x_16*x_34 + \n"
                       "        x_17*x_34 + x_18*x_34 + x_23*x_34 + x_27*x_34 + x_32*x_34 + x_6*x_35 + x_10*x_35 + x_11*x_35 + x_12*x_35 + x_13*x_35 + x_20*x_35 + x_21*x_35 + x_22*x_35 + x_23*x_35 + x_24*x_35 + x_25*x_35 + \n"
                       "        x_26*x_35 + x_28*x_35 + x_29*x_35 + x_30*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_34*x_35 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_8*x_36 + x_10*x_36 + x_11*x_36 + x_19*x_36 + x_22*x_36\n"
                       "        + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_30*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_11*x_37 + x_15*x_37 + x_16*x_37\n"
                       "        + x_17*x_37 + x_20*x_37 + x_22*x_37 + x_23*x_37 + x_24*x_37 + x_26*x_37 + x_32*x_37 + x_34*x_37 + x_36*x_37 + x_5*x_38 + x_9*x_38 + x_14*x_38 + x_16*x_38 + x_17*x_38 + x_19*x_38 + x_20*x_38 + \n"
                       "        x_24*x_38 + x_26*x_38 + x_29*x_38 + x_33*x_38 + x_34*x_38 + x_36*x_38 + x_37*x_38 + x_4*x_39 + x_8*x_39 + x_13*x_39 + x_14*x_39 + x_15*x_39 + x_16*x_39 + x_17*x_39 + x_19*x_39 + x_22*x_39 + \n"
                       "        x_25*x_39 + x_30*x_39 + x_31*x_39 + x_35*x_39 + x_36*x_39 + x_3*x_40 + x_4*x_40 + x_6*x_40 + x_7*x_40 + x_10*x_40 + x_12*x_40 + x_16*x_40 + x_18*x_40 + x_19*x_40 + x_21*x_40 + x_22*x_40 + \n"
                       "        x_24*x_40 + x_25*x_40 + x_27*x_40 + x_29*x_40 + x_30*x_40 + x_32*x_40 + x_33*x_40 + x_36*x_40 + x_1*x_41 + x_3*x_41 + x_5*x_41 + x_6*x_41 + x_14*x_41 + x_15*x_41 + x_18*x_41 + x_20*x_41 + \n"
                       "        x_21*x_41 + x_22*x_41 + x_23*x_41 + x_25*x_41 + x_26*x_41 + x_30*x_41 + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_37*x_41 + x_4*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_8*x_42 + x_13*x_42 + x_14*x_42\n"
                       "        + x_18*x_42 + x_23*x_42 + x_27*x_42 + x_29*x_42 + x_30*x_42 + x_34*x_42 + x_36*x_42 + x_37*x_42 + x_40*x_42 + x_41*x_42 + x_1*x_43 + x_2*x_43 + x_3*x_43 + x_6*x_43 + x_8*x_43 + x_13*x_43 + \n"
                       "        x_14*x_43 + x_19*x_43 + x_20*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + \n"
                       "        x_42*x_43 + x_1*x_44 + x_3*x_44 + x_6*x_44 + x_8*x_44 + x_9*x_44 + x_11*x_44 + x_14*x_44 + x_15*x_44 + x_17*x_44 + x_20*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_27*x_44 + x_28*x_44 + x_30*x_44\n"
                       "        + x_31*x_44 + x_32*x_44 + x_34*x_44 + x_37*x_44 + x_39*x_44 + x_41*x_44 + x_42*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + x_3*x_45 + x_4*x_45 + x_8*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + \n"
                       "        x_15*x_45 + x_17*x_45 + x_20*x_45 + x_22*x_45 + x_23*x_45 + x_26*x_45 + x_29*x_45 + x_35*x_45 + x_39*x_45 + x_42*x_45 + x_43*x_45 + x_44*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_14*x_46 + \n"
                       "        x_16*x_46 + x_17*x_46 + x_18*x_46 + x_19*x_46 + x_21*x_46 + x_26*x_46 + x_27*x_46 + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_32*x_46 + x_34*x_46 + x_37*x_46 + x_38*x_46 + x_39*x_46 + x_40*x_46 + \n"
                       "        x_43*x_46 + x_3*x_47 + x_9*x_47 + x_12*x_47 + x_13*x_47 + x_16*x_47 + x_17*x_47 + x_18*x_47 + x_20*x_47 + x_21*x_47 + x_23*x_47 + x_26*x_47 + x_27*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + \n"
                       "        x_33*x_47 + x_34*x_47 + x_35*x_47 + x_37*x_47 + x_42*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_46*x_47 + x_2*x_48 + x_4*x_48 + x_5*x_48 + x_9*x_48 + x_12*x_48 + x_13*x_48 + x_14*x_48 + \n"
                       "        x_18*x_48 + x_20*x_48 + x_24*x_48 + x_25*x_48 + x_27*x_48 + x_30*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_35*x_48 + x_36*x_48 + x_37*x_48 + x_38*x_48 + x_43*x_48 + x_46*x_48 + x_1*x_49 + \n"
                       "        x_6*x_49 + x_7*x_49 + x_8*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 + x_14*x_49 + x_16*x_49 + x_17*x_49 + x_19*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_30*x_49 + x_32*x_49 + \n"
                       "        x_33*x_49 + x_38*x_49 + x_40*x_49 + x_48*x_49 + x_2 + x_4 + x_5 + x_6 + x_8 + x_9 + x_15 + x_17 + x_18 + x_19 + x_20 + x_22 + x_25 + x_26 + x_29 + x_31 + x_32 + x_34 + x_35 + x_37 + x_38 + x_44 + \n"
                       "        x_47,\n"
                       "    x_1*x_2 + x_2*x_3 + x_3*x_4 + x_2*x_5 + x_2*x_6 + x_5*x_6 + x_2*x_7 + x_5*x_7 + x_6*x_7 + x_3*x_8 + x_4*x_8 + x_6*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_5*x_9 + x_7*x_9 + x_8*x_9 + x_1*x_10 + x_2*x_10 \n"
                       "        + x_3*x_10 + x_4*x_10 + x_5*x_10 + x_8*x_10 + x_9*x_10 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_6*x_11 + x_9*x_11 + x_1*x_12 + x_6*x_12 + x_7*x_12 + x_10*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + \n"
                       "        x_4*x_13 + x_12*x_13 + x_1*x_14 + x_3*x_14 + x_7*x_14 + x_8*x_14 + x_9*x_14 + x_10*x_14 + x_11*x_14 + x_13*x_14 + x_2*x_15 + x_3*x_15 + x_8*x_15 + x_9*x_15 + x_1*x_16 + x_4*x_16 + x_7*x_16 + \n"
                       "        x_9*x_16 + x_10*x_16 + x_12*x_16 + x_14*x_16 + x_15*x_16 + x_1*x_17 + x_2*x_17 + x_4*x_17 + x_5*x_17 + x_7*x_17 + x_8*x_17 + x_9*x_17 + x_10*x_17 + x_11*x_17 + x_12*x_17 + x_13*x_17 + x_14*x_17 + \n"
                       "        x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_9*x_18 + x_10*x_18 + x_1*x_19 + x_2*x_19 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_7*x_19 + x_8*x_19 + x_9*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + \n"
                       "        x_14*x_19 + x_17*x_19 + x_18*x_19 + x_2*x_20 + x_4*x_20 + x_7*x_20 + x_9*x_20 + x_10*x_20 + x_15*x_20 + x_17*x_20 + x_1*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_10*x_21 + x_14*x_21 + x_15*x_21 + \n"
                       "        x_16*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_1*x_22 + x_5*x_22 + x_6*x_22 + x_7*x_22 + x_8*x_22 + x_10*x_22 + x_11*x_22 + x_13*x_22 + x_14*x_22 + x_15*x_22 + x_16*x_22 + x_19*x_22 + x_20*x_22\n"
                       "        + x_21*x_22 + x_1*x_23 + x_4*x_23 + x_9*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_15*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_21*x_23 + x_3*x_24 + x_4*x_24 + x_5*x_24 + x_6*x_24\n"
                       "        + x_8*x_24 + x_10*x_24 + x_11*x_24 + x_13*x_24 + x_14*x_24 + x_16*x_24 + x_17*x_24 + x_19*x_24 + x_20*x_24 + x_22*x_24 + x_23*x_24 + x_3*x_25 + x_4*x_25 + x_5*x_25 + x_6*x_25 + x_10*x_25 + \n"
                       "        x_11*x_25 + x_15*x_25 + x_17*x_25 + x_21*x_25 + x_22*x_25 + x_24*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_4*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_11*x_26 + x_13*x_26 + x_14*x_26 + \n"
                       "        x_15*x_26 + x_18*x_26 + x_22*x_26 + x_23*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_4*x_27 + x_5*x_27 + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_10*x_27 + x_12*x_27 + x_14*x_27 + x_15*x_27 + x_16*x_27 +\n"
                       "        x_17*x_27 + x_19*x_27 + x_21*x_27 + x_24*x_27 + x_4*x_28 + x_6*x_28 + x_7*x_28 + x_9*x_28 + x_11*x_28 + x_16*x_28 + x_18*x_28 + x_21*x_28 + x_22*x_28 + x_26*x_28 + x_27*x_28 + x_1*x_29 + x_3*x_29 \n"
                       "        + x_4*x_29 + x_5*x_29 + x_6*x_29 + x_7*x_29 + x_9*x_29 + x_11*x_29 + x_14*x_29 + x_15*x_29 + x_19*x_29 + x_22*x_29 + x_23*x_29 + x_24*x_29 + x_26*x_29 + x_6*x_30 + x_10*x_30 + x_12*x_30 + \n"
                       "        x_21*x_30 + x_26*x_30 + x_27*x_30 + x_1*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_9*x_31 + x_11*x_31 + x_15*x_31 + x_19*x_31 + x_20*x_31 + x_21*x_31 + x_22*x_31 + x_26*x_31 + x_1*x_32 +\n"
                       "        x_4*x_32 + x_10*x_32 + x_12*x_32 + x_13*x_32 + x_16*x_32 + x_17*x_32 + x_18*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_3*x_33 + x_6*x_33 + \n"
                       "        x_8*x_33 + x_9*x_33 + x_12*x_33 + x_13*x_33 + x_15*x_33 + x_16*x_33 + x_18*x_33 + x_19*x_33 + x_22*x_33 + x_24*x_33 + x_26*x_33 + x_29*x_33 + x_31*x_33 + x_1*x_34 + x_2*x_34 + x_3*x_34 + x_4*x_34 \n"
                       "        + x_9*x_34 + x_10*x_34 + x_13*x_34 + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_20*x_34 + x_22*x_34 + x_23*x_34 + x_25*x_34 + x_26*x_34 + x_27*x_34 + x_30*x_34 + x_31*x_34 + x_1*x_35 + x_2*x_35 + \n"
                       "        x_4*x_35 + x_5*x_35 + x_6*x_35 + x_8*x_35 + x_11*x_35 + x_13*x_35 + x_14*x_35 + x_15*x_35 + x_16*x_35 + x_20*x_35 + x_21*x_35 + x_24*x_35 + x_25*x_35 + x_27*x_35 + x_28*x_35 + x_31*x_35 + x_1*x_36\n"
                       "        + x_2*x_36 + x_6*x_36 + x_7*x_36 + x_12*x_36 + x_14*x_36 + x_15*x_36 + x_19*x_36 + x_20*x_36 + x_21*x_36 + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + \n"
                       "        x_30*x_36 + x_31*x_36 + x_33*x_36 + x_34*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 + x_13*x_37 + x_14*x_37 + x_15*x_37 + x_20*x_37 \n"
                       "        + x_22*x_37 + x_28*x_37 + x_32*x_37 + x_33*x_37 + x_2*x_38 + x_7*x_38 + x_9*x_38 + x_11*x_38 + x_13*x_38 + x_15*x_38 + x_17*x_38 + x_19*x_38 + x_21*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + \n"
                       "        x_25*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_31*x_38 + x_34*x_38 + x_36*x_38 + x_1*x_39 + x_2*x_39 + x_6*x_39 + x_7*x_39 + x_9*x_39 + x_11*x_39 + x_12*x_39 + x_13*x_39 + x_15*x_39 + x_17*x_39\n"
                       "        + x_18*x_39 + x_21*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + x_26*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_33*x_39 + x_34*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + \n"
                       "        x_3*x_40 + x_5*x_40 + x_7*x_40 + x_8*x_40 + x_10*x_40 + x_13*x_40 + x_16*x_40 + x_18*x_40 + x_21*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_29*x_40 + x_30*x_40 + x_31*x_40 + x_33*x_40 + \n"
                       "        x_35*x_40 + x_37*x_40 + x_38*x_40 + x_1*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_12*x_41 + x_16*x_41 + x_17*x_41 + x_21*x_41 + x_25*x_41 + x_27*x_41 + x_32*x_41 + x_35*x_41 + x_36*x_41 + \n"
                       "        x_37*x_41 + x_38*x_41 + x_39*x_41 + x_1*x_42 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_8*x_42 + x_9*x_42 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_15*x_42 + x_16*x_42 + x_21*x_42 + x_25*x_42 + x_26*x_42 \n"
                       "        + x_27*x_42 + x_28*x_42 + x_33*x_42 + x_36*x_42 + x_41*x_42 + x_1*x_43 + x_4*x_43 + x_6*x_43 + x_8*x_43 + x_10*x_43 + x_11*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + \n"
                       "        x_18*x_43 + x_20*x_43 + x_21*x_43 + x_22*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 + x_27*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_36*x_43 + x_38*x_43 + x_40*x_43 + \n"
                       "        x_41*x_43 + x_42*x_43 + x_1*x_44 + x_2*x_44 + x_5*x_44 + x_6*x_44 + x_7*x_44 + x_8*x_44 + x_9*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_17*x_44 + x_21*x_44 +\n"
                       "        x_22*x_44 + x_25*x_44 + x_27*x_44 + x_28*x_44 + x_29*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_37*x_44 + x_40*x_44 + x_1*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_11*x_45 + x_13*x_45 + \n"
                       "        x_14*x_45 + x_15*x_45 + x_16*x_45 + x_21*x_45 + x_22*x_45 + x_26*x_45 + x_29*x_45 + x_33*x_45 + x_35*x_45 + x_39*x_45 + x_42*x_45 + x_44*x_45 + x_3*x_46 + x_7*x_46 + x_8*x_46 + x_10*x_46 + \n"
                       "        x_14*x_46 + x_17*x_46 + x_18*x_46 + x_21*x_46 + x_24*x_46 + x_25*x_46 + x_29*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_39*x_46 + x_40*x_46 + x_42*x_46 + x_1*x_47 + x_3*x_47 + x_5*x_47 + \n"
                       "        x_10*x_47 + x_11*x_47 + x_16*x_47 + x_20*x_47 + x_24*x_47 + x_26*x_47 + x_27*x_47 + x_30*x_47 + x_31*x_47 + x_33*x_47 + x_35*x_47 + x_36*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + x_41*x_47 + \n"
                       "        x_44*x_47 + x_45*x_47 + x_46*x_47 + x_2*x_48 + x_3*x_48 + x_4*x_48 + x_6*x_48 + x_7*x_48 + x_8*x_48 + x_12*x_48 + x_13*x_48 + x_14*x_48 + x_16*x_48 + x_18*x_48 + x_21*x_48 + x_22*x_48 + x_23*x_48 \n"
                       "        + x_26*x_48 + x_28*x_48 + x_31*x_48 + x_33*x_48 + x_35*x_48 + x_36*x_48 + x_39*x_48 + x_41*x_48 + x_42*x_48 + x_43*x_48 + x_46*x_48 + x_1*x_49 + x_2*x_49 + x_3*x_49 + x_5*x_49 + x_6*x_49 + \n"
                       "        x_7*x_49 + x_9*x_49 + x_10*x_49 + x_12*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_18*x_49 + x_19*x_49 + x_22*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_32*x_49 + x_33*x_49 + x_36*x_49 + \n"
                       "        x_38*x_49 + x_40*x_49 + x_41*x_49 + x_42*x_49 + x_45*x_49 + x_48*x_49 + x_3 + x_4 + x_8 + x_10 + x_12 + x_14 + x_17 + x_18 + x_21 + x_22 + x_27 + x_28 + x_29 + x_31 + x_33 + x_36 + x_38 + x_40 + \n"
                       "        x_42 + x_46 + x_47 + x_49,\n"
                       "    x_1*x_2 + x_1*x_4 + x_2*x_4 + x_2*x_5 + x_1*x_6 + x_2*x_7 + x_3*x_7 + x_1*x_8 + x_2*x_8 + x_5*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_1*x_10 + x_4*x_10 + x_5*x_10\n"
                       "        + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_2*x_11 + x_3*x_11 + x_5*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_1*x_12 + x_4*x_12 + x_5*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + \n"
                       "        x_3*x_13 + x_5*x_13 + x_8*x_13 + x_9*x_13 + x_10*x_13 + x_11*x_13 + x_4*x_14 + x_9*x_14 + x_10*x_14 + x_12*x_14 + x_2*x_15 + x_3*x_15 + x_6*x_15 + x_9*x_15 + x_11*x_15 + x_12*x_15 + x_13*x_15 + \n"
                       "        x_14*x_15 + x_1*x_16 + x_2*x_16 + x_5*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_12*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_6*x_17 + x_7*x_17 + x_9*x_17 + x_12*x_17 + x_15*x_17 + \n"
                       "        x_16*x_17 + x_3*x_18 + x_4*x_18 + x_6*x_18 + x_7*x_18 + x_8*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + x_17*x_18 + x_2*x_19 + x_4*x_19 + x_5*x_19 + \n"
                       "        x_6*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_14*x_19 + x_17*x_19 + x_2*x_20 + x_3*x_20 + x_6*x_20 + x_7*x_20 + x_9*x_20 + x_12*x_20 + x_15*x_20 + x_1*x_21 + x_2*x_21 + x_4*x_21 + x_5*x_21 + \n"
                       "        x_6*x_21 + x_7*x_21 + x_10*x_21 + x_12*x_21 + x_14*x_21 + x_16*x_21 + x_18*x_21 + x_1*x_22 + x_2*x_22 + x_4*x_22 + x_6*x_22 + x_7*x_22 + x_10*x_22 + x_12*x_22 + x_18*x_22 + x_20*x_22 + x_21*x_22 +\n"
                       "        x_1*x_23 + x_2*x_23 + x_3*x_23 + x_6*x_23 + x_7*x_23 + x_10*x_23 + x_13*x_23 + x_14*x_23 + x_20*x_23 + x_22*x_23 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_8*x_24 + x_9*x_24 + x_12*x_24 + x_13*x_24 + \n"
                       "        x_14*x_24 + x_15*x_24 + x_16*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_1*x_25 + x_3*x_25 + x_5*x_25 + x_7*x_25 + x_9*x_25 + x_10*x_25 + x_13*x_25 + x_19*x_25 + x_21*x_25 + x_22*x_25 + x_1*x_26 \n"
                       "        + x_2*x_26 + x_7*x_26 + x_8*x_26 + x_10*x_26 + x_12*x_26 + x_14*x_26 + x_16*x_26 + x_17*x_26 + x_19*x_26 + x_20*x_26 + x_21*x_26 + x_23*x_26 + x_24*x_26 + x_1*x_27 + x_2*x_27 + x_3*x_27 + x_5*x_27\n"
                       "        + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_11*x_27 + x_14*x_27 + x_15*x_27 + x_16*x_27 + x_19*x_27 + x_22*x_27 + x_24*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_8*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 \n"
                       "        + x_13*x_28 + x_16*x_28 + x_22*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_27*x_28 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_6*x_29 + x_10*x_29 + x_12*x_29 + x_14*x_29 + x_15*x_29 + \n"
                       "        x_16*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_22*x_29 + x_24*x_29 + x_26*x_29 + x_4*x_30 + x_7*x_30 + x_8*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + \n"
                       "        x_19*x_30 + x_20*x_30 + x_21*x_30 + x_22*x_30 + x_23*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + x_29*x_30 + x_4*x_31 + x_6*x_31 + x_9*x_31 + x_17*x_31 + x_20*x_31 + x_23*x_31 + x_24*x_31 + \n"
                       "        x_28*x_31 + x_1*x_32 + x_5*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_22*x_32 + x_23*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + \n"
                       "        x_28*x_32 + x_29*x_32 + x_1*x_33 + x_6*x_33 + x_10*x_33 + x_11*x_33 + x_12*x_33 + x_13*x_33 + x_14*x_33 + x_15*x_33 + x_16*x_33 + x_17*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_26*x_33 + \n"
                       "        x_27*x_33 + x_28*x_33 + x_29*x_33 + x_31*x_33 + x_32*x_33 + x_1*x_34 + x_8*x_34 + x_9*x_34 + x_10*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_22*x_34 + \n"
                       "        x_25*x_34 + x_26*x_34 + x_28*x_34 + x_29*x_34 + x_31*x_34 + x_32*x_34 + x_3*x_35 + x_7*x_35 + x_11*x_35 + x_12*x_35 + x_15*x_35 + x_16*x_35 + x_17*x_35 + x_20*x_35 + x_21*x_35 + x_22*x_35 + \n"
                       "        x_24*x_35 + x_27*x_35 + x_30*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_2*x_36 + x_5*x_36 + x_8*x_36 + x_11*x_36 + x_12*x_36 + x_14*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + x_19*x_36 + \n"
                       "        x_20*x_36 + x_21*x_36 + x_29*x_36 + x_33*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_5*x_37 + x_9*x_37 + x_11*x_37 + x_13*x_37 + x_14*x_37 + x_15*x_37 + x_17*x_37 + x_18*x_37 + x_19*x_37\n"
                       "        + x_20*x_37 + x_21*x_37 + x_26*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + x_1*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_13*x_38 + x_15*x_38 + x_16*x_38 + \n"
                       "        x_22*x_38 + x_23*x_38 + x_24*x_38 + x_25*x_38 + x_26*x_38 + x_27*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_7*x_39 + x_10*x_39 + x_11*x_39 + x_15*x_39 + \n"
                       "        x_19*x_39 + x_21*x_39 + x_22*x_39 + x_25*x_39 + x_28*x_39 + x_31*x_39 + x_32*x_39 + x_35*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_5*x_40 + x_9*x_40 + x_11*x_40 + x_12*x_40\n"
                       "        + x_17*x_40 + x_18*x_40 + x_19*x_40 + x_23*x_40 + x_25*x_40 + x_26*x_40 + x_30*x_40 + x_35*x_40 + x_38*x_40 + x_2*x_41 + x_3*x_41 + x_4*x_41 + x_5*x_41 + x_9*x_41 + x_12*x_41 + x_13*x_41 + \n"
                       "        x_14*x_41 + x_15*x_41 + x_16*x_41 + x_19*x_41 + x_21*x_41 + x_27*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_32*x_41 + x_36*x_41 + x_37*x_41 + x_38*x_41 + x_3*x_42 + x_6*x_42 + x_8*x_42 + \n"
                       "        x_10*x_42 + x_12*x_42 + x_13*x_42 + x_14*x_42 + x_15*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_20*x_42 + x_23*x_42 + x_25*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_32*x_42 + \n"
                       "        x_35*x_42 + x_36*x_42 + x_37*x_42 + x_3*x_43 + x_4*x_43 + x_5*x_43 + x_6*x_43 + x_7*x_43 + x_8*x_43 + x_12*x_43 + x_16*x_43 + x_17*x_43 + x_18*x_43 + x_19*x_43 + x_20*x_43 + x_22*x_43 + x_26*x_43 \n"
                       "        + x_29*x_43 + x_30*x_43 + x_32*x_43 + x_34*x_43 + x_38*x_43 + x_41*x_43 + x_1*x_44 + x_3*x_44 + x_4*x_44 + x_8*x_44 + x_15*x_44 + x_16*x_44 + x_17*x_44 + x_18*x_44 + x_19*x_44 + x_20*x_44 + \n"
                       "        x_22*x_44 + x_23*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_30*x_44 + x_32*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_37*x_44 + x_39*x_44 + x_40*x_44 + x_42*x_44 + x_43*x_44 + x_4*x_45 + \n"
                       "        x_7*x_45 + x_8*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_19*x_45 + x_20*x_45 + x_23*x_45 + x_24*x_45 + x_25*x_45 + x_26*x_45 + x_29*x_45 + x_33*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + \n"
                       "        x_42*x_45 + x_44*x_45 + x_1*x_46 + x_3*x_46 + x_4*x_46 + x_10*x_46 + x_13*x_46 + x_17*x_46 + x_19*x_46 + x_20*x_46 + x_21*x_46 + x_22*x_46 + x_23*x_46 + x_30*x_46 + x_32*x_46 + x_33*x_46 + \n"
                       "        x_34*x_46 + x_37*x_46 + x_38*x_46 + x_40*x_46 + x_42*x_46 + x_43*x_46 + x_45*x_46 + x_3*x_47 + x_7*x_47 + x_8*x_47 + x_10*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_16*x_47 + x_18*x_47 + \n"
                       "        x_20*x_47 + x_21*x_47 + x_22*x_47 + x_25*x_47 + x_27*x_47 + x_31*x_47 + x_35*x_47 + x_36*x_47 + x_37*x_47 + x_41*x_47 + x_42*x_47 + x_44*x_47 + x_1*x_48 + x_2*x_48 + x_4*x_48 + x_8*x_48 + \n"
                       "        x_10*x_48 + x_11*x_48 + x_12*x_48 + x_15*x_48 + x_21*x_48 + x_23*x_48 + x_25*x_48 + x_26*x_48 + x_27*x_48 + x_29*x_48 + x_32*x_48 + x_33*x_48 + x_34*x_48 + x_35*x_48 + x_36*x_48 + x_40*x_48 + \n"
                       "        x_41*x_48 + x_43*x_48 + x_44*x_48 + x_46*x_48 + x_5*x_49 + x_6*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_13*x_49 + x_20*x_49 + x_21*x_49 + x_22*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49\n"
                       "        + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_31*x_49 + x_39*x_49 + x_40*x_49 + x_43*x_49 + x_44*x_49 + x_48*x_49 + x_2 + x_7 + x_8 + x_10 + x_11 + x_13 + x_16 + x_17 + x_19 + x_23 + x_26 + x_27 + x_28 \n"
                       "        + x_30 + x_35 + x_36 + x_37 + x_38 + x_39 + x_40 + x_43 + x_44 + x_47,\n"
                       "    x_1*x_2 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_1*x_7 + x_3*x_7 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_6*x_9 + x_7*x_9 + \n"
                       "        x_3*x_10 + x_6*x_10 + x_8*x_10 + x_9*x_10 + x_2*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_10*x_12 + x_4*x_13 + x_10*x_13 + x_11*x_13 + x_2*x_14 + x_3*x_14 + \n"
                       "        x_5*x_14 + x_8*x_14 + x_11*x_14 + x_1*x_15 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_8*x_15 + x_13*x_15 + x_14*x_15 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_8*x_16 + x_10*x_16 + x_13*x_16 + x_1*x_17 + \n"
                       "        x_2*x_17 + x_3*x_17 + x_7*x_17 + x_9*x_17 + x_11*x_17 + x_12*x_17 + x_14*x_17 + x_15*x_17 + x_1*x_18 + x_5*x_18 + x_8*x_18 + x_10*x_18 + x_12*x_18 + x_13*x_18 + x_17*x_18 + x_2*x_19 + x_5*x_19 + \n"
                       "        x_8*x_19 + x_9*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_14*x_19 + x_16*x_19 + x_2*x_20 + x_4*x_20 + x_8*x_20 + x_10*x_20 + x_11*x_20 + x_13*x_20 + x_14*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20\n"
                       "        + x_1*x_21 + x_5*x_21 + x_6*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_12*x_21 + x_17*x_21 + x_18*x_21 + x_2*x_22 + x_9*x_22 + x_14*x_22 + x_15*x_22 + x_19*x_22 + x_20*x_22 + x_1*x_23 + x_3*x_23 +\n"
                       "        x_4*x_23 + x_6*x_23 + x_14*x_23 + x_17*x_23 + x_20*x_23 + x_21*x_23 + x_22*x_23 + x_1*x_24 + x_3*x_24 + x_4*x_24 + x_8*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_14*x_24 + x_15*x_24 + x_19*x_24 \n"
                       "        + x_21*x_24 + x_2*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 + x_13*x_25 + x_14*x_25 + x_16*x_25 + x_17*x_25 + x_18*x_25 + x_22*x_25 + x_23*x_25 + x_24*x_25 + x_2*x_26 + x_6*x_26 + x_7*x_26 + \n"
                       "        x_9*x_26 + x_11*x_26 + x_17*x_26 + x_19*x_26 + x_22*x_26 + x_2*x_27 + x_12*x_27 + x_13*x_27 + x_16*x_27 + x_18*x_27 + x_19*x_27 + x_20*x_27 + x_25*x_27 + x_26*x_27 + x_3*x_28 + x_5*x_28 + x_6*x_28\n"
                       "        + x_9*x_28 + x_10*x_28 + x_14*x_28 + x_15*x_28 + x_17*x_28 + x_19*x_28 + x_21*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_27*x_28 + x_3*x_29 + x_4*x_29 + x_7*x_29 + x_11*x_29 + x_13*x_29 + \n"
                       "        x_14*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_26*x_29 + x_27*x_29 + x_2*x_30 + x_5*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_10*x_30 + x_12*x_30 + x_13*x_30 + x_15*x_30\n"
                       "        + x_16*x_30 + x_17*x_30 + x_18*x_30 + x_20*x_30 + x_21*x_30 + x_24*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + x_29*x_30 + x_2*x_31 + x_6*x_31 + x_8*x_31 + x_12*x_31 + x_14*x_31 + x_15*x_31 + \n"
                       "        x_19*x_31 + x_22*x_31 + x_25*x_31 + x_26*x_31 + x_28*x_31 + x_29*x_31 + x_3*x_32 + x_4*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_16*x_32 + x_19*x_32 + x_21*x_32\n"
                       "        + x_22*x_32 + x_24*x_32 + x_25*x_32 + x_30*x_32 + x_1*x_33 + x_3*x_33 + x_7*x_33 + x_9*x_33 + x_11*x_33 + x_13*x_33 + x_16*x_33 + x_22*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_30*x_33 + \n"
                       "        x_32*x_33 + x_1*x_34 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_16*x_34 + x_22*x_34 + x_23*x_34 + x_24*x_34 + \n"
                       "        x_26*x_34 + x_31*x_34 + x_2*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_10*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_15*x_35 + x_18*x_35 + x_19*x_35 + x_23*x_35 + x_27*x_35 + x_28*x_35 + \n"
                       "        x_30*x_35 + x_31*x_35 + x_33*x_35 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_9*x_36 + x_14*x_36 + x_16*x_36 + x_18*x_36 + x_23*x_36 + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_29*x_36 + x_30*x_36\n"
                       "        + x_34*x_36 + x_4*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_14*x_37 + x_16*x_37 + x_18*x_37 + x_20*x_37 + x_24*x_37 + x_29*x_37 + x_31*x_37 + x_33*x_37 + x_34*x_37 + x_35*x_37 + \n"
                       "        x_5*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_13*x_38 + x_14*x_38 + x_20*x_38 + x_22*x_38 + x_25*x_38 + x_28*x_38 + x_31*x_38 + x_32*x_38 + x_35*x_38 + x_36*x_38 + x_1*x_39 + x_2*x_39 + \n"
                       "        x_4*x_39 + x_6*x_39 + x_7*x_39 + x_8*x_39 + x_9*x_39 + x_12*x_39 + x_15*x_39 + x_16*x_39 + x_18*x_39 + x_19*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_24*x_39 + x_25*x_39 + x_26*x_39 + x_27*x_39\n"
                       "        + x_29*x_39 + x_31*x_39 + x_32*x_39 + x_34*x_39 + x_37*x_39 + x_38*x_39 + x_1*x_40 + x_5*x_40 + x_6*x_40 + x_8*x_40 + x_11*x_40 + x_14*x_40 + x_17*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + \n"
                       "        x_22*x_40 + x_25*x_40 + x_27*x_40 + x_29*x_40 + x_30*x_40 + x_32*x_40 + x_34*x_40 + x_35*x_40 + x_37*x_40 + x_38*x_40 + x_39*x_40 + x_1*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_11*x_41\n"
                       "        + x_12*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41 + x_25*x_41 + x_26*x_41 + x_30*x_41 + x_36*x_41 + x_39*x_41 + x_40*x_41 + x_3*x_42 + x_7*x_42 + x_10*x_42 + \n"
                       "        x_14*x_42 + x_15*x_42 + x_16*x_42 + x_17*x_42 + x_19*x_42 + x_20*x_42 + x_21*x_42 + x_24*x_42 + x_26*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_37*x_42 + x_38*x_42 + x_39*x_42 + x_40*x_42 + \n"
                       "        x_41*x_42 + x_4*x_43 + x_8*x_43 + x_10*x_43 + x_15*x_43 + x_20*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + x_26*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + \n"
                       "        x_40*x_43 + x_41*x_43 + x_1*x_44 + x_2*x_44 + x_4*x_44 + x_8*x_44 + x_10*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_16*x_44 + x_17*x_44 + x_18*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44 + \n"
                       "        x_27*x_44 + x_28*x_44 + x_32*x_44 + x_34*x_44 + x_36*x_44 + x_37*x_44 + x_38*x_44 + x_39*x_44 + x_40*x_44 + x_41*x_44 + x_43*x_44 + x_1*x_45 + x_3*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 \n"
                       "        + x_9*x_45 + x_11*x_45 + x_13*x_45 + x_16*x_45 + x_17*x_45 + x_22*x_45 + x_23*x_45 + x_28*x_45 + x_30*x_45 + x_31*x_45 + x_32*x_45 + x_33*x_45 + x_35*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + \n"
                       "        x_40*x_45 + x_41*x_45 + x_43*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_5*x_46 + x_7*x_46 + x_8*x_46 + x_9*x_46 + x_13*x_46 + x_14*x_46 + x_15*x_46 + x_18*x_46 + x_20*x_46 + x_23*x_46 + x_25*x_46 +\n"
                       "        x_26*x_46 + x_32*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_37*x_46 + x_38*x_46 + x_39*x_46 + x_43*x_46 + x_7*x_47 + x_11*x_47 + x_12*x_47 + x_13*x_47 + x_20*x_47 + x_22*x_47 + x_24*x_47 + \n"
                       "        x_26*x_47 + x_27*x_47 + x_29*x_47 + x_31*x_47 + x_36*x_47 + x_41*x_47 + x_43*x_47 + x_3*x_48 + x_4*x_48 + x_6*x_48 + x_8*x_48 + x_10*x_48 + x_12*x_48 + x_15*x_48 + x_23*x_48 + x_24*x_48 + \n"
                       "        x_26*x_48 + x_29*x_48 + x_34*x_48 + x_37*x_48 + x_38*x_48 + x_40*x_48 + x_41*x_48 + x_43*x_48 + x_46*x_48 + x_47*x_48 + x_2*x_49 + x_4*x_49 + x_5*x_49 + x_6*x_49 + x_7*x_49 + x_10*x_49 + x_11*x_49\n"
                       "        + x_12*x_49 + x_15*x_49 + x_16*x_49 + x_19*x_49 + x_21*x_49 + x_24*x_49 + x_26*x_49 + x_32*x_49 + x_33*x_49 + x_37*x_49 + x_39*x_49 + x_40*x_49 + x_43*x_49 + x_44*x_49 + x_45*x_49 + x_46*x_49 + \n"
                       "        x_47*x_49 + x_1 + x_2 + x_3 + x_7 + x_8 + x_9 + x_10 + x_14 + x_15 + x_16 + x_19 + x_21 + x_22 + x_23 + x_24 + x_29 + x_33 + x_36 + x_37 + x_38 + x_39 + x_40 + x_42 + x_45 + x_47 + x_48,\n"
                       "    x_1*x_2 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_3*x_6 + x_5*x_6 + x_1*x_7 + x_3*x_7 + x_4*x_7 + x_5*x_7 + x_2*x_8 + x_4*x_8 + x_6*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_8*x_9 + \n"
                       "        x_1*x_10 + x_2*x_10 + x_4*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_2*x_11 + x_4*x_11 + x_8*x_11 + x_10*x_11 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_8*x_12 + x_9*x_12 + \n"
                       "        x_10*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_4*x_13 + x_5*x_13 + x_6*x_13 + x_8*x_13 + x_10*x_13 + x_1*x_14 + x_3*x_14 + x_5*x_14 + x_6*x_14 + x_10*x_14 + x_11*x_14 + x_12*x_14 + x_3*x_15 + \n"
                       "        x_4*x_15 + x_5*x_15 + x_6*x_15 + x_7*x_15 + x_11*x_15 + x_12*x_15 + x_13*x_15 + x_2*x_16 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_6*x_16 + x_10*x_16 + x_14*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + \n"
                       "        x_7*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_6*x_18 + x_7*x_18 + x_8*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_16*x_18 + x_2*x_19 + x_3*x_19 + \n"
                       "        x_4*x_19 + x_8*x_19 + x_10*x_19 + x_11*x_19 + x_13*x_19 + x_14*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_18*x_19 + x_1*x_20 + x_2*x_20 + x_5*x_20 + x_12*x_20 + x_14*x_20 + x_16*x_20 + x_17*x_20\n"
                       "        + x_19*x_20 + x_2*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_11*x_21 + x_13*x_21 + x_16*x_21 + x_17*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_4*x_22 \n"
                       "        + x_5*x_22 + x_7*x_22 + x_8*x_22 + x_9*x_22 + x_10*x_22 + x_11*x_22 + x_12*x_22 + x_15*x_22 + x_17*x_22 + x_2*x_23 + x_3*x_23 + x_4*x_23 + x_6*x_23 + x_7*x_23 + x_8*x_23 + x_9*x_23 + x_12*x_23 + \n"
                       "        x_16*x_23 + x_17*x_23 + x_18*x_23 + x_22*x_23 + x_1*x_24 + x_2*x_24 + x_12*x_24 + x_13*x_24 + x_14*x_24 + x_15*x_24 + x_17*x_24 + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + \n"
                       "        x_1*x_25 + x_4*x_25 + x_6*x_25 + x_7*x_25 + x_9*x_25 + x_12*x_25 + x_13*x_25 + x_15*x_25 + x_16*x_25 + x_18*x_25 + x_20*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + \n"
                       "        x_4*x_26 + x_7*x_26 + x_8*x_26 + x_10*x_26 + x_11*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_18*x_26 + x_2*x_27 + x_4*x_27 + x_7*x_27 + x_8*x_27 + x_11*x_27 + x_13*x_27 + x_15*x_27 + x_18*x_27 +\n"
                       "        x_19*x_27 + x_20*x_27 + x_25*x_27 + x_3*x_28 + x_4*x_28 + x_9*x_28 + x_10*x_28 + x_12*x_28 + x_14*x_28 + x_16*x_28 + x_18*x_28 + x_19*x_28 + x_20*x_28 + x_21*x_28 + x_22*x_28 + x_24*x_28 + \n"
                       "        x_27*x_28 + x_3*x_29 + x_6*x_29 + x_9*x_29 + x_11*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_3*x_30 + x_4*x_30 + x_5*x_30 + x_7*x_30 + x_9*x_30 + x_10*x_30 + \n"
                       "        x_12*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_16*x_30 + x_17*x_30 + x_19*x_30 + x_20*x_30 + x_24*x_30 + x_25*x_30 + x_28*x_30 + x_2*x_31 + x_3*x_31 + x_5*x_31 + x_6*x_31 + x_10*x_31 + \n"
                       "        x_13*x_31 + x_14*x_31 + x_15*x_31 + x_17*x_31 + x_18*x_31 + x_19*x_31 + x_20*x_31 + x_22*x_31 + x_23*x_31 + x_24*x_31 + x_26*x_31 + x_27*x_31 + x_1*x_32 + x_2*x_32 + x_3*x_32 + x_4*x_32 + x_6*x_32\n"
                       "        + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_15*x_32 + x_16*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32 + x_26*x_32 + x_28*x_32 + x_30*x_32 + x_31*x_32 + \n"
                       "        x_1*x_33 + x_7*x_33 + x_9*x_33 + x_11*x_33 + x_15*x_33 + x_16*x_33 + x_20*x_33 + x_23*x_33 + x_24*x_33 + x_26*x_33 + x_28*x_33 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_10*x_34 + x_11*x_34 +\n"
                       "        x_13*x_34 + x_14*x_34 + x_17*x_34 + x_20*x_34 + x_21*x_34 + x_22*x_34 + x_23*x_34 + x_24*x_34 + x_26*x_34 + x_27*x_34 + x_28*x_34 + x_1*x_35 + x_2*x_35 + x_6*x_35 + x_9*x_35 + x_11*x_35 + \n"
                       "        x_13*x_35 + x_15*x_35 + x_16*x_35 + x_17*x_35 + x_20*x_35 + x_23*x_35 + x_28*x_35 + x_29*x_35 + x_31*x_35 + x_32*x_35 + x_5*x_36 + x_6*x_36 + x_7*x_36 + x_11*x_36 + x_15*x_36 + x_16*x_36 + \n"
                       "        x_17*x_36 + x_18*x_36 + x_19*x_36 + x_20*x_36 + x_22*x_36 + x_24*x_36 + x_30*x_36 + x_31*x_36 + x_33*x_36 + x_2*x_37 + x_4*x_37 + x_5*x_37 + x_7*x_37 + x_9*x_37 + x_10*x_37 + x_14*x_37 + x_15*x_37\n"
                       "        + x_18*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 + x_22*x_37 + x_23*x_37 + x_24*x_37 + x_25*x_37 + x_26*x_37 + x_31*x_37 + x_32*x_37 + x_35*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + \n"
                       "        x_6*x_38 + x_8*x_38 + x_9*x_38 + x_10*x_38 + x_13*x_38 + x_15*x_38 + x_17*x_38 + x_19*x_38 + x_20*x_38 + x_22*x_38 + x_23*x_38 + x_27*x_38 + x_33*x_38 + x_34*x_38 + x_35*x_38 + x_37*x_38 + \n"
                       "        x_1*x_39 + x_2*x_39 + x_6*x_39 + x_7*x_39 + x_8*x_39 + x_10*x_39 + x_13*x_39 + x_16*x_39 + x_22*x_39 + x_23*x_39 + x_25*x_39 + x_26*x_39 + x_28*x_39 + x_33*x_39 + x_36*x_39 + x_37*x_39 + x_2*x_40 \n"
                       "        + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_8*x_40 + x_10*x_40 + x_14*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + x_22*x_40 + x_23*x_40 + x_27*x_40 + x_28*x_40 + x_30*x_40 + x_31*x_40 + x_32*x_40 + \n"
                       "        x_36*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_11*x_41 + x_13*x_41 + x_14*x_41 + x_16*x_41 + x_18*x_41 + x_22*x_41 + x_23*x_41 + x_25*x_41 +\n"
                       "        x_27*x_41 + x_28*x_41 + x_30*x_41 + x_33*x_41 + x_34*x_41 + x_2*x_42 + x_3*x_42 + x_4*x_42 + x_7*x_42 + x_8*x_42 + x_10*x_42 + x_13*x_42 + x_15*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42\n"
                       "        + x_20*x_42 + x_21*x_42 + x_22*x_42 + x_24*x_42 + x_26*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_36*x_42 + x_40*x_42 + x_1*x_43 + x_6*x_43 + x_9*x_43 + x_10*x_43 + \n"
                       "        x_14*x_43 + x_18*x_43 + x_20*x_43 + x_21*x_43 + x_23*x_43 + x_24*x_43 + x_26*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 + x_32*x_43 + x_34*x_43 + x_35*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + \n"
                       "        x_40*x_43 + x_7*x_44 + x_8*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_16*x_44 + x_22*x_44 + x_23*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_29*x_44 + \n"
                       "        x_32*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_39*x_44 + x_42*x_44 + x_3*x_45 + x_6*x_45 + x_8*x_45 + x_9*x_45 + x_10*x_45 + x_18*x_45 + x_19*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + \n"
                       "        x_29*x_45 + x_34*x_45 + x_35*x_45 + x_38*x_45 + x_40*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_3*x_46 + x_13*x_46 + x_18*x_46 + x_19*x_46 + x_21*x_46 + x_24*x_46 + x_28*x_46 + \n"
                       "        x_31*x_46 + x_37*x_46 + x_38*x_46 + x_39*x_46 + x_41*x_46 + x_42*x_46 + x_44*x_46 + x_2*x_47 + x_5*x_47 + x_6*x_47 + x_10*x_47 + x_11*x_47 + x_15*x_47 + x_20*x_47 + x_21*x_47 + x_22*x_47 + \n"
                       "        x_28*x_47 + x_29*x_47 + x_33*x_47 + x_35*x_47 + x_37*x_47 + x_39*x_47 + x_40*x_47 + x_41*x_47 + x_43*x_47 + x_44*x_47 + x_1*x_48 + x_2*x_48 + x_4*x_48 + x_8*x_48 + x_9*x_48 + x_10*x_48 + x_12*x_48\n"
                       "        + x_13*x_48 + x_15*x_48 + x_16*x_48 + x_19*x_48 + x_22*x_48 + x_23*x_48 + x_27*x_48 + x_28*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_36*x_48 + x_40*x_48 + x_42*x_48 + x_43*x_48 + x_45*x_48 + \n"
                       "        x_2*x_49 + x_3*x_49 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_10*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_17*x_49 + x_18*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49 + x_23*x_49 + x_25*x_49 \n"
                       "        + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_33*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + x_40*x_49 + x_41*x_49 + x_43*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + x_1 + x_3 + x_8 + x_9 + x_10 + x_12 + \n"
                       "        x_13 + x_14 + x_16 + x_19 + x_21 + x_22 + x_24 + x_28 + x_29 + x_32 + x_33 + x_35 + x_36 + x_37 + x_38 + x_43 + x_45 + x_47 + x_48 + x_49,\n"
                       "    x_1*x_2 + x_1*x_3 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_2*x_6 + x_4*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_3*x_7 + x_4*x_7 + x_5*x_7 + x_6*x_7 + x_2*x_8 + x_4*x_8 + \n"
                       "        x_2*x_9 + x_3*x_9 + x_5*x_9 + x_6*x_9 + x_2*x_10 + x_5*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_6*x_11 + x_9*x_11 + x_10*x_11 + x_1*x_12 + x_6*x_12 + x_7*x_12 + x_10*x_12 + x_11*x_12 +\n"
                       "        x_2*x_13 + x_3*x_13 + x_4*x_13 + x_5*x_13 + x_6*x_13 + x_8*x_13 + x_9*x_13 + x_10*x_13 + x_1*x_14 + x_3*x_14 + x_4*x_14 + x_5*x_14 + x_8*x_14 + x_9*x_14 + x_10*x_14 + x_11*x_14 + x_12*x_14 + \n"
                       "        x_13*x_14 + x_1*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + x_8*x_15 + x_9*x_15 + x_11*x_15 + x_12*x_15 + x_13*x_15 + x_4*x_16 + x_5*x_16 + x_7*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_4*x_17 + \n"
                       "        x_5*x_17 + x_6*x_17 + x_8*x_17 + x_13*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_7*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_14*x_18 + x_15*x_18 + x_16*x_18 + x_1*x_19 + \n"
                       "        x_2*x_19 + x_4*x_19 + x_5*x_19 + x_7*x_19 + x_8*x_19 + x_10*x_19 + x_12*x_19 + x_15*x_19 + x_17*x_19 + x_3*x_20 + x_6*x_20 + x_9*x_20 + x_12*x_20 + x_13*x_20 + x_16*x_20 + x_18*x_20 + x_19*x_20 + \n"
                       "        x_1*x_21 + x_2*x_21 + x_6*x_21 + x_8*x_21 + x_10*x_21 + x_11*x_21 + x_14*x_21 + x_16*x_21 + x_19*x_21 + x_20*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_4*x_22 + x_6*x_22 + x_7*x_22 + x_10*x_22 + \n"
                       "        x_15*x_22 + x_16*x_22 + x_18*x_22 + x_20*x_22 + x_1*x_23 + x_2*x_23 + x_3*x_23 + x_4*x_23 + x_5*x_23 + x_6*x_23 + x_11*x_23 + x_12*x_23 + x_15*x_23 + x_16*x_23 + x_17*x_23 + x_19*x_23 + x_20*x_23 \n"
                       "        + x_1*x_24 + x_4*x_24 + x_5*x_24 + x_6*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_13*x_24 + x_14*x_24 + x_15*x_24 + x_16*x_24 + x_17*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_1*x_25 + x_3*x_25\n"
                       "        + x_5*x_25 + x_6*x_25 + x_7*x_25 + x_8*x_25 + x_10*x_25 + x_11*x_25 + x_14*x_25 + x_15*x_25 + x_18*x_25 + x_19*x_25 + x_20*x_25 + x_21*x_25 + x_24*x_25 + x_2*x_26 + x_4*x_26 + x_5*x_26 + x_7*x_26 \n"
                       "        + x_8*x_26 + x_10*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_18*x_26 + x_19*x_26 + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_23*x_26 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_8*x_27 + x_9*x_27 + \n"
                       "        x_11*x_27 + x_13*x_27 + x_14*x_27 + x_18*x_27 + x_22*x_27 + x_23*x_27 + x_24*x_27 + x_26*x_27 + x_1*x_28 + x_3*x_28 + x_4*x_28 + x_7*x_28 + x_8*x_28 + x_9*x_28 + x_10*x_28 + x_12*x_28 + x_14*x_28 \n"
                       "        + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_21*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_26*x_28 + x_27*x_28 + x_3*x_29 + x_5*x_29 + x_8*x_29 + x_9*x_29 + x_10*x_29 + x_13*x_29 + x_14*x_29 + \n"
                       "        x_18*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_23*x_29 + x_24*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 + x_3*x_30 + x_4*x_30 + x_7*x_30 + x_9*x_30 + x_10*x_30 + x_11*x_30 \n"
                       "        + x_12*x_30 + x_13*x_30 + x_15*x_30 + x_16*x_30 + x_17*x_30 + x_18*x_30 + x_19*x_30 + x_22*x_30 + x_23*x_30 + x_25*x_30 + x_28*x_30 + x_29*x_30 + x_2*x_31 + x_4*x_31 + x_5*x_31 + x_10*x_31 + \n"
                       "        x_13*x_31 + x_15*x_31 + x_19*x_31 + x_20*x_31 + x_22*x_31 + x_3*x_32 + x_4*x_32 + x_7*x_32 + x_10*x_32 + x_11*x_32 + x_13*x_32 + x_14*x_32 + x_16*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + \n"
                       "        x_23*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_2*x_33 + x_4*x_33 + x_7*x_33 + x_9*x_33 + x_10*x_33 + x_11*x_33 + x_12*x_33 + x_14*x_33 + x_19*x_33\n"
                       "        + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_23*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_31*x_33 + x_1*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_15*x_34 + \n"
                       "        x_16*x_34 + x_18*x_34 + x_24*x_34 + x_28*x_34 + x_1*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_12*x_35 + x_14*x_35 + x_15*x_35 + x_16*x_35 + x_18*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35 + \n"
                       "        x_22*x_35 + x_25*x_35 + x_29*x_35 + x_31*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_5*x_36 + x_7*x_36 + x_11*x_36 + x_13*x_36 + x_15*x_36 + x_17*x_36 + x_19*x_36 + x_21*x_36\n"
                       "        + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_29*x_36 + x_31*x_36 + x_32*x_36 + x_2*x_37 + x_5*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_16*x_37 + x_18*x_37 + x_22*x_37 + \n"
                       "        x_24*x_37 + x_25*x_37 + x_26*x_37 + x_27*x_37 + x_31*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + x_36*x_37 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + x_10*x_38 + x_12*x_38 + x_14*x_38 + \n"
                       "        x_16*x_38 + x_19*x_38 + x_20*x_38 + x_22*x_38 + x_26*x_38 + x_28*x_38 + x_30*x_38 + x_35*x_38 + x_36*x_38 + x_37*x_38 + x_1*x_39 + x_2*x_39 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_13*x_39 + x_15*x_39\n"
                       "        + x_19*x_39 + x_20*x_39 + x_21*x_39 + x_23*x_39 + x_25*x_39 + x_27*x_39 + x_28*x_39 + x_30*x_39 + x_32*x_39 + x_33*x_39 + x_34*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_4*x_40 + \n"
                       "        x_5*x_40 + x_8*x_40 + x_13*x_40 + x_14*x_40 + x_19*x_40 + x_20*x_40 + x_24*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + x_30*x_40 + x_36*x_40 + x_38*x_40 + x_39*x_40 + x_3*x_41 + x_7*x_41 + \n"
                       "        x_10*x_41 + x_12*x_41 + x_13*x_41 + x_16*x_41 + x_17*x_41 + x_18*x_41 + x_20*x_41 + x_22*x_41 + x_24*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + x_31*x_41 + x_33*x_41 + x_37*x_41 + x_38*x_41 + \n"
                       "        x_39*x_41 + x_2*x_42 + x_4*x_42 + x_5*x_42 + x_7*x_42 + x_10*x_42 + x_14*x_42 + x_15*x_42 + x_17*x_42 + x_19*x_42 + x_21*x_42 + x_22*x_42 + x_23*x_42 + x_24*x_42 + x_25*x_42 + x_30*x_42 + \n"
                       "        x_31*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_39*x_42 + x_40*x_42 + x_41*x_42 + x_1*x_43 + x_2*x_43 + x_4*x_43 + x_5*x_43 + x_7*x_43 + x_9*x_43 + x_13*x_43 + x_16*x_43 + x_17*x_43 + x_21*x_43 \n"
                       "        + x_26*x_43 + x_29*x_43 + x_30*x_43 + x_32*x_43 + x_33*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_40*x_43 + x_41*x_43 + x_42*x_43 + x_4*x_44 + x_12*x_44 + x_16*x_44 + x_18*x_44 + \n"
                       "        x_19*x_44 + x_20*x_44 + x_23*x_44 + x_30*x_44 + x_31*x_44 + x_32*x_44 + x_35*x_44 + x_37*x_44 + x_39*x_44 + x_41*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + x_4*x_45 + x_11*x_45 + x_17*x_45 + \n"
                       "        x_20*x_45 + x_22*x_45 + x_25*x_45 + x_26*x_45 + x_27*x_45 + x_29*x_45 + x_32*x_45 + x_34*x_45 + x_35*x_45 + x_37*x_45 + x_39*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_1*x_46 + x_13*x_46 + \n"
                       "        x_14*x_46 + x_16*x_46 + x_18*x_46 + x_19*x_46 + x_20*x_46 + x_22*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_27*x_46 + x_33*x_46 + x_36*x_46 + x_37*x_46 + x_38*x_46 + x_40*x_46 + x_43*x_46 + \n"
                       "        x_44*x_46 + x_1*x_47 + x_2*x_47 + x_3*x_47 + x_4*x_47 + x_5*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_15*x_47 + x_16*x_47 + x_17*x_47 + x_19*x_47 + x_22*x_47 + x_23*x_47\n"
                       "        + x_24*x_47 + x_28*x_47 + x_29*x_47 + x_31*x_47 + x_32*x_47 + x_33*x_47 + x_35*x_47 + x_36*x_47 + x_39*x_47 + x_40*x_47 + x_41*x_47 + x_44*x_47 + x_46*x_47 + x_3*x_48 + x_4*x_48 + x_7*x_48 + \n"
                       "        x_8*x_48 + x_11*x_48 + x_13*x_48 + x_15*x_48 + x_19*x_48 + x_20*x_48 + x_22*x_48 + x_23*x_48 + x_24*x_48 + x_28*x_48 + x_30*x_48 + x_32*x_48 + x_34*x_48 + x_37*x_48 + x_38*x_48 + x_40*x_48 + \n"
                       "        x_42*x_48 + x_45*x_48 + x_47*x_48 + x_1*x_49 + x_2*x_49 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_9*x_49 + x_12*x_49 + x_15*x_49 + x_16*x_49 + x_17*x_49 + x_18*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49 \n"
                       "        + x_27*x_49 + x_28*x_49 + x_30*x_49 + x_32*x_49 + x_33*x_49 + x_34*x_49 + x_36*x_49 + x_38*x_49 + x_39*x_49 + x_42*x_49 + x_43*x_49 + x_44*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + \n"
                       "        x_1 + x_2 + x_7 + x_8 + x_9 + x_13 + x_15 + x_17 + x_21 + x_22 + x_23 + x_24 + x_26 + x_27 + x_28 + x_29 + x_32 + x_34 + x_35 + x_37 + x_38 + x_39 + x_42 + x_43 + x_44 + x_45 + x_46 + x_47,\n"
                       "    x_3*x_4 + x_1*x_5 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + x_5*x_7 + x_3*x_8 + x_4*x_8 + x_3*x_9 + x_1*x_10 + x_2*x_10 + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_3*x_11 + \n"
                       "        x_6*x_11 + x_7*x_11 + x_9*x_11 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_4*x_13 + x_7*x_13 + x_9*x_13 + \n"
                       "        x_12*x_13 + x_1*x_14 + x_4*x_14 + x_12*x_14 + x_2*x_15 + x_5*x_15 + x_7*x_15 + x_8*x_15 + x_10*x_15 + x_11*x_15 + x_12*x_15 + x_1*x_16 + x_3*x_16 + x_8*x_16 + x_10*x_16 + x_12*x_16 + x_13*x_16 + \n"
                       "        x_14*x_16 + x_15*x_16 + x_3*x_17 + x_5*x_17 + x_9*x_17 + x_11*x_17 + x_12*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_5*x_18 + x_10*x_18 + x_13*x_18 + x_14*x_18 + x_16*x_18 +\n"
                       "        x_1*x_19 + x_2*x_19 + x_3*x_19 + x_5*x_19 + x_6*x_19 + x_7*x_19 + x_8*x_19 + x_11*x_19 + x_13*x_19 + x_16*x_19 + x_18*x_19 + x_1*x_20 + x_3*x_20 + x_5*x_20 + x_7*x_20 + x_8*x_20 + x_11*x_20 + \n"
                       "        x_13*x_20 + x_14*x_20 + x_19*x_20 + x_1*x_21 + x_4*x_21 + x_8*x_21 + x_9*x_21 + x_11*x_21 + x_12*x_21 + x_1*x_22 + x_4*x_22 + x_5*x_22 + x_8*x_22 + x_10*x_22 + x_11*x_22 + x_14*x_22 + x_15*x_22 + \n"
                       "        x_18*x_22 + x_20*x_22 + x_1*x_23 + x_4*x_23 + x_6*x_23 + x_8*x_23 + x_10*x_23 + x_12*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_20*x_23 + x_1*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_8*x_24 + \n"
                       "        x_10*x_24 + x_11*x_24 + x_12*x_24 + x_15*x_24 + x_16*x_24 + x_17*x_24 + x_18*x_24 + x_19*x_24 + x_22*x_24 + x_1*x_25 + x_5*x_25 + x_6*x_25 + x_11*x_25 + x_12*x_25 + x_16*x_25 + x_17*x_25 + \n"
                       "        x_20*x_25 + x_22*x_25 + x_24*x_25 + x_3*x_26 + x_8*x_26 + x_9*x_26 + x_10*x_26 + x_11*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_19*x_26 + x_20*x_26 + x_22*x_26 + \n"
                       "        x_24*x_26 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_10*x_27 + x_12*x_27 + x_16*x_27 + x_17*x_27 + x_18*x_27 + x_20*x_27 + x_23*x_27 + x_26*x_27 + x_3*x_28 + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_8*x_28 + \n"
                       "        x_10*x_28 + x_11*x_28 + x_12*x_28 + x_14*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_20*x_28 + x_22*x_28 + x_27*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_7*x_29 + x_8*x_29 + x_12*x_29 + x_14*x_29\n"
                       "        + x_15*x_29 + x_16*x_29 + x_18*x_29 + x_19*x_29 + x_22*x_29 + x_24*x_29 + x_25*x_29 + x_26*x_29 + x_27*x_29 + x_2*x_30 + x_3*x_30 + x_4*x_30 + x_7*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + \n"
                       "        x_14*x_30 + x_19*x_30 + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_26*x_30 + x_29*x_30 + x_1*x_31 + x_3*x_31 + x_7*x_31 + x_9*x_31 + x_10*x_31 + x_12*x_31 + x_15*x_31 + x_18*x_31 + x_19*x_31 + \n"
                       "        x_21*x_31 + x_22*x_31 + x_26*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32 + x_4*x_32 + x_5*x_32 + x_7*x_32 + x_8*x_32 + x_12*x_32 + x_14*x_32 + x_17*x_32 + x_18*x_32 + x_22*x_32 + x_23*x_32 + x_24*x_32 \n"
                       "        + x_31*x_32 + x_1*x_33 + x_3*x_33 + x_4*x_33 + x_5*x_33 + x_7*x_33 + x_8*x_33 + x_9*x_33 + x_12*x_33 + x_13*x_33 + x_14*x_33 + x_16*x_33 + x_17*x_33 + x_18*x_33 + x_19*x_33 + x_20*x_33 + x_23*x_33\n"
                       "        + x_24*x_33 + x_25*x_33 + x_27*x_33 + x_5*x_34 + x_7*x_34 + x_9*x_34 + x_10*x_34 + x_11*x_34 + x_14*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_21*x_34 + x_26*x_34 + x_28*x_34 + x_29*x_34 + \n"
                       "        x_30*x_34 + x_2*x_35 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35 + x_23*x_35 + x_25*x_35 \n"
                       "        + x_27*x_35 + x_34*x_35 + x_2*x_36 + x_4*x_36 + x_5*x_36 + x_7*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_14*x_36 + x_16*x_36 + x_17*x_36 + x_26*x_36 + x_27*x_36 + x_29*x_36 + x_32*x_36 + \n"
                       "        x_1*x_37 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_7*x_37 + x_8*x_37 + x_13*x_37 + x_14*x_37 + x_16*x_37 + x_18*x_37 + x_25*x_37 + x_26*x_37 + x_30*x_37 + x_32*x_37 + x_33*x_37 + x_35*x_37 + x_36*x_37 \n"
                       "        + x_4*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + x_11*x_38 + x_12*x_38 + x_14*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_18*x_38 + x_19*x_38 + x_20*x_38 + x_25*x_38 + x_27*x_38 + \n"
                       "        x_28*x_38 + x_29*x_38 + x_30*x_38 + x_32*x_38 + x_36*x_38 + x_1*x_39 + x_4*x_39 + x_12*x_39 + x_16*x_39 + x_22*x_39 + x_24*x_39 + x_25*x_39 + x_26*x_39 + x_27*x_39 + x_28*x_39 + x_30*x_39 + \n"
                       "        x_31*x_39 + x_32*x_39 + x_33*x_39 + x_34*x_39 + x_35*x_39 + x_36*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_5*x_40 + x_6*x_40 + x_8*x_40 + x_11*x_40 + x_12*x_40 + x_13*x_40 + x_14*x_40 + x_18*x_40 \n"
                       "        + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_27*x_40 + x_28*x_40 + x_34*x_40 + x_37*x_40 + x_39*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_5*x_41 + x_6*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41\n"
                       "        + x_11*x_41 + x_12*x_41 + x_15*x_41 + x_17*x_41 + x_24*x_41 + x_26*x_41 + x_27*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_40*x_41 + x_1*x_42 + \n"
                       "        x_2*x_42 + x_3*x_42 + x_6*x_42 + x_11*x_42 + x_14*x_42 + x_18*x_42 + x_19*x_42 + x_21*x_42 + x_22*x_42 + x_24*x_42 + x_29*x_42 + x_32*x_42 + x_34*x_42 + x_35*x_42 + x_36*x_42 + x_37*x_42 + \n"
                       "        x_38*x_42 + x_39*x_42 + x_40*x_42 + x_41*x_42 + x_2*x_43 + x_8*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_17*x_43 + x_20*x_43 + x_22*x_43 + x_23*x_43 + x_26*x_43 + x_27*x_43 + \n"
                       "        x_28*x_43 + x_29*x_43 + x_30*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_42*x_43 + x_1*x_44 + x_3*x_44 + x_5*x_44 + x_6*x_44 + x_7*x_44 + x_9*x_44 + x_10*x_44 + x_17*x_44 + x_22*x_44 \n"
                       "        + x_23*x_44 + x_26*x_44 + x_28*x_44 + x_30*x_44 + x_33*x_44 + x_34*x_44 + x_37*x_44 + x_38*x_44 + x_39*x_44 + x_41*x_44 + x_1*x_45 + x_3*x_45 + x_4*x_45 + x_6*x_45 + x_9*x_45 + x_12*x_45 + \n"
                       "        x_14*x_45 + x_15*x_45 + x_17*x_45 + x_18*x_45 + x_20*x_45 + x_21*x_45 + x_26*x_45 + x_28*x_45 + x_31*x_45 + x_32*x_45 + x_34*x_45 + x_35*x_45 + x_40*x_45 + x_43*x_45 + x_44*x_45 + x_3*x_46 + \n"
                       "        x_5*x_46 + x_6*x_46 + x_7*x_46 + x_8*x_46 + x_9*x_46 + x_10*x_46 + x_11*x_46 + x_12*x_46 + x_14*x_46 + x_15*x_46 + x_17*x_46 + x_20*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_27*x_46\n"
                       "        + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_33*x_46 + x_35*x_46 + x_36*x_46 + x_37*x_46 + x_38*x_46 + x_40*x_46 + x_44*x_46 + x_1*x_47 + x_5*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_12*x_47 + \n"
                       "        x_13*x_47 + x_15*x_47 + x_18*x_47 + x_19*x_47 + x_20*x_47 + x_21*x_47 + x_23*x_47 + x_24*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_31*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_36*x_47 + \n"
                       "        x_37*x_47 + x_38*x_47 + x_40*x_47 + x_41*x_47 + x_42*x_47 + x_43*x_47 + x_45*x_47 + x_1*x_48 + x_2*x_48 + x_6*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_16*x_48 + x_19*x_48 + x_21*x_48 + \n"
                       "        x_22*x_48 + x_28*x_48 + x_29*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_36*x_48 + x_37*x_48 + x_38*x_48 + x_41*x_48 + x_44*x_48 + x_47*x_48 + x_1*x_49 + x_2*x_49 + x_3*x_49 + x_4*x_49 + x_8*x_49\n"
                       "        + x_12*x_49 + x_17*x_49 + x_18*x_49 + x_19*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_33*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + x_37*x_49 + x_38*x_49 + x_40*x_49 + \n"
                       "        x_45*x_49 + x_47*x_49 + x_48*x_49 + x_3 + x_4 + x_5 + x_6 + x_8 + x_9 + x_10 + x_12 + x_15 + x_17 + x_19 + x_21 + x_23 + x_24 + x_26 + x_27 + x_28 + x_31 + x_32 + x_33 + x_34 + x_35 + x_38 + x_40 \n"
                       "        + x_41 + x_45 + x_48 + x_49,\n"
                       "    x_2*x_3 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_3*x_7 + x_4*x_7 + x_6*x_7 + x_1*x_8 + x_2*x_8 + x_3*x_8 + x_5*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + \n"
                       "        x_7*x_9 + x_1*x_10 + x_3*x_10 + x_5*x_10 + x_9*x_10 + x_2*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_8*x_11 + x_1*x_12 + x_4*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_9*x_12 + x_2*x_13 + x_3*x_13 +\n"
                       "        x_4*x_13 + x_5*x_13 + x_7*x_13 + x_9*x_13 + x_11*x_13 + x_12*x_13 + x_2*x_14 + x_3*x_14 + x_9*x_14 + x_11*x_14 + x_1*x_15 + x_3*x_15 + x_8*x_15 + x_10*x_15 + x_11*x_15 + x_13*x_15 + x_4*x_16 + \n"
                       "        x_5*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_14*x_16 + x_15*x_16 + x_2*x_17 + x_5*x_17 + x_6*x_17 + x_14*x_17 + x_15*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + \n"
                       "        x_11*x_18 + x_12*x_18 + x_14*x_18 + x_16*x_18 + x_17*x_18 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_6*x_19 + x_10*x_19 + x_11*x_19 + x_13*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_18*x_19 + x_3*x_20\n"
                       "        + x_4*x_20 + x_5*x_20 + x_6*x_20 + x_9*x_20 + x_10*x_20 + x_12*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_18*x_21 + \n"
                       "        x_19*x_21 + x_1*x_22 + x_2*x_22 + x_4*x_22 + x_5*x_22 + x_6*x_22 + x_7*x_22 + x_9*x_22 + x_12*x_22 + x_15*x_22 + x_16*x_22 + x_17*x_22 + x_18*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_3*x_23 + \n"
                       "        x_4*x_23 + x_8*x_23 + x_9*x_23 + x_10*x_23 + x_13*x_23 + x_14*x_23 + x_15*x_23 + x_18*x_23 + x_20*x_23 + x_21*x_23 + x_1*x_24 + x_8*x_24 + x_11*x_24 + x_15*x_24 + x_18*x_24 + x_20*x_24 + x_21*x_24\n"
                       "        + x_22*x_24 + x_23*x_24 + x_1*x_25 + x_3*x_25 + x_4*x_25 + x_6*x_25 + x_8*x_25 + x_14*x_25 + x_16*x_25 + x_17*x_25 + x_18*x_25 + x_19*x_25 + x_21*x_25 + x_23*x_25 + x_24*x_25 + x_1*x_26 + x_2*x_26\n"
                       "        + x_3*x_26 + x_4*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_12*x_26 + x_14*x_26 + x_17*x_26 + x_18*x_26 + x_19*x_26 + x_22*x_26 + x_4*x_27 + x_5*x_27 + x_7*x_27 + x_8*x_27 + x_11*x_27 + x_13*x_27 +\n"
                       "        x_14*x_27 + x_16*x_27 + x_17*x_27 + x_19*x_27 + x_23*x_27 + x_25*x_27 + x_3*x_28 + x_4*x_28 + x_14*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_21*x_28 + x_22*x_28 + x_24*x_28 + \n"
                       "        x_26*x_28 + x_27*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_7*x_29 + x_9*x_29 + x_13*x_29 + x_16*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 + x_23*x_29 + x_24*x_29 + x_25*x_29 + x_27*x_29 \n"
                       "        + x_2*x_30 + x_4*x_30 + x_5*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_13*x_30 + x_15*x_30 + x_19*x_30 + x_20*x_30 + x_21*x_30 + x_22*x_30 + x_24*x_30 + x_26*x_30 + x_28*x_30 + x_4*x_31 \n"
                       "        + x_6*x_31 + x_10*x_31 + x_11*x_31 + x_14*x_31 + x_16*x_31 + x_17*x_31 + x_21*x_31 + x_22*x_31 + x_23*x_31 + x_25*x_31 + x_26*x_31 + x_27*x_31 + x_29*x_31 + x_5*x_32 + x_10*x_32 + x_17*x_32 + \n"
                       "        x_19*x_32 + x_20*x_32 + x_22*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32 + x_29*x_32 + x_2*x_33 + x_5*x_33 + x_6*x_33 + x_12*x_33 + x_13*x_33 + x_15*x_33 + x_16*x_33 + x_22*x_33 + x_25*x_33 + \n"
                       "        x_26*x_33 + x_29*x_33 + x_30*x_33 + x_31*x_33 + x_1*x_34 + x_2*x_34 + x_3*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_20*x_34 + x_21*x_34 + x_25*x_34 +\n"
                       "        x_29*x_34 + x_30*x_34 + x_32*x_34 + x_5*x_35 + x_6*x_35 + x_11*x_35 + x_12*x_35 + x_14*x_35 + x_15*x_35 + x_16*x_35 + x_17*x_35 + x_20*x_35 + x_21*x_35 + x_25*x_35 + x_27*x_35 + x_28*x_35 + \n"
                       "        x_30*x_35 + x_31*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_5*x_36 + x_7*x_36 + x_8*x_36 + x_12*x_36 + x_15*x_36 + x_17*x_36 + x_19*x_36 + x_22*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 \n"
                       "        + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_29*x_36 + x_31*x_36 + x_33*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_5*x_37 + x_7*x_37 + x_9*x_37 + x_11*x_37 + x_12*x_37 + \n"
                       "        x_13*x_37 + x_15*x_37 + x_17*x_37 + x_18*x_37 + x_22*x_37 + x_26*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + x_33*x_37 + x_36*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_5*x_38\n"
                       "        + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_11*x_38 + x_12*x_38 + x_14*x_38 + x_16*x_38 + x_17*x_38 + x_21*x_38 + x_22*x_38 + x_25*x_38 + x_28*x_38 + x_33*x_38 + x_35*x_38 + x_2*x_39 + x_3*x_39 + \n"
                       "        x_5*x_39 + x_7*x_39 + x_8*x_39 + x_10*x_39 + x_13*x_39 + x_15*x_39 + x_17*x_39 + x_18*x_39 + x_19*x_39 + x_21*x_39 + x_23*x_39 + x_25*x_39 + x_26*x_39 + x_27*x_39 + x_28*x_39 + x_29*x_39 + \n"
                       "        x_30*x_39 + x_31*x_39 + x_32*x_39 + x_33*x_39 + x_38*x_39 + x_4*x_40 + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_8*x_40 + x_9*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_17*x_40 + x_18*x_40 + x_21*x_40 \n"
                       "        + x_24*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + x_34*x_40 + x_35*x_40 + x_36*x_40 + x_38*x_40 + x_2*x_41 + x_3*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_10*x_41 + x_13*x_41 + \n"
                       "        x_14*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_23*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_35*x_41 + x_36*x_41 + x_37*x_41 + x_1*x_42 + x_3*x_42 + \n"
                       "        x_4*x_42 + x_7*x_42 + x_8*x_42 + x_11*x_42 + x_12*x_42 + x_14*x_42 + x_16*x_42 + x_21*x_42 + x_24*x_42 + x_26*x_42 + x_28*x_42 + x_32*x_42 + x_35*x_42 + x_37*x_42 + x_38*x_42 + x_39*x_42 + \n"
                       "        x_41*x_42 + x_1*x_43 + x_2*x_43 + x_3*x_43 + x_4*x_43 + x_7*x_43 + x_8*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_19*x_43 + x_20*x_43 + x_22*x_43 + x_24*x_43 + x_25*x_43 \n"
                       "        + x_26*x_43 + x_29*x_43 + x_30*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_37*x_43 + x_42*x_43 + x_2*x_44 + x_5*x_44 + x_6*x_44 + x_10*x_44 + x_12*x_44 + x_15*x_44 + x_17*x_44 + x_19*x_44 + \n"
                       "        x_21*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_26*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_38*x_44 + x_41*x_44 + x_43*x_44 + x_2*x_45 + x_4*x_45 + x_6*x_45 + x_10*x_45 + \n"
                       "        x_12*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + x_18*x_45 + x_20*x_45 + x_21*x_45 + x_26*x_45 + x_28*x_45 + x_31*x_45 + x_32*x_45 + x_35*x_45 + x_36*x_45 + x_38*x_45 + x_40*x_45 + x_41*x_45 + \n"
                       "        x_42*x_45 + x_1*x_46 + x_2*x_46 + x_8*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + x_15*x_46 + x_16*x_46 + x_17*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_28*x_46 + x_29*x_46 + x_36*x_46 + \n"
                       "        x_38*x_46 + x_40*x_46 + x_43*x_46 + x_2*x_47 + x_3*x_47 + x_6*x_47 + x_7*x_47 + x_13*x_47 + x_21*x_47 + x_25*x_47 + x_26*x_47 + x_27*x_47 + x_31*x_47 + x_32*x_47 + x_34*x_47 + x_36*x_47 + \n"
                       "        x_37*x_47 + x_38*x_47 + x_39*x_47 + x_43*x_47 + x_46*x_47 + x_1*x_48 + x_2*x_48 + x_3*x_48 + x_4*x_48 + x_5*x_48 + x_9*x_48 + x_11*x_48 + x_14*x_48 + x_17*x_48 + x_18*x_48 + x_20*x_48 + x_21*x_48 \n"
                       "        + x_25*x_48 + x_26*x_48 + x_31*x_48 + x_32*x_48 + x_34*x_48 + x_36*x_48 + x_37*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_42*x_48 + x_43*x_48 + x_44*x_48 + x_47*x_48 + x_1*x_49 + x_2*x_49 + \n"
                       "        x_3*x_49 + x_5*x_49 + x_10*x_49 + x_17*x_49 + x_18*x_49 + x_20*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_34*x_49 + x_35*x_49 + \n"
                       "        x_37*x_49 + x_39*x_49 + x_41*x_49 + x_42*x_49 + x_43*x_49 + x_44*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_1 + x_2 + x_5 + x_6 + x_8 + x_13 + x_14 + x_16 + x_18 + x_19 + x_20 + x_25\n"
                       "        + x_26 + x_29 + x_30 + x_32 + x_35 + x_36 + x_39 + x_41 + x_43 + x_46 + x_48,\n"
                       "    x_1*x_2 + x_1*x_3 + x_2*x_3 + x_2*x_4 + x_1*x_5 + x_3*x_5 + x_3*x_6 + x_4*x_6 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_5*x_8 + x_7*x_8 + x_1*x_9 + x_3*x_9 + x_4*x_9 + x_6*x_9 + x_2*x_10 + x_3*x_10 + x_6*x_10\n"
                       "        + x_8*x_10 + x_3*x_11 + x_4*x_11 + x_7*x_11 + x_8*x_11 + x_9*x_11 + x_4*x_12 + x_6*x_12 + x_7*x_12 + x_9*x_12 + x_11*x_12 + x_2*x_13 + x_5*x_13 + x_6*x_13 + x_7*x_13 + x_8*x_13 + x_9*x_13 + \n"
                       "        x_11*x_13 + x_12*x_13 + x_1*x_14 + x_4*x_14 + x_6*x_14 + x_7*x_14 + x_8*x_14 + x_10*x_14 + x_11*x_14 + x_12*x_14 + x_1*x_15 + x_3*x_15 + x_7*x_15 + x_9*x_15 + x_10*x_15 + x_2*x_16 + x_5*x_16 + \n"
                       "        x_6*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_12*x_16 + x_14*x_16 + x_15*x_16 + x_1*x_17 + x_2*x_17 + x_3*x_17 + x_5*x_17 + x_6*x_17 + x_7*x_17 + x_8*x_17 + x_13*x_17 + x_16*x_17 + \n"
                       "        x_1*x_18 + x_5*x_18 + x_8*x_18 + x_9*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_14*x_18 + x_15*x_18 + x_3*x_19 + x_4*x_19 + x_6*x_19 + x_9*x_19 + x_11*x_19 + x_7*x_20 + x_9*x_20 + x_11*x_20 + \n"
                       "        x_12*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 + x_19*x_20 + x_4*x_21 + x_6*x_21 + x_8*x_21 + x_10*x_21 + x_17*x_21 + x_18*x_21 + x_1*x_22 + x_5*x_22 + x_7*x_22 + x_8*x_22 +\n"
                       "        x_10*x_22 + x_15*x_22 + x_16*x_22 + x_17*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_2*x_23 + x_4*x_23 + x_5*x_23 + x_8*x_23 + x_11*x_23 + x_16*x_23 + x_17*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23\n"
                       "        + x_21*x_23 + x_22*x_23 + x_1*x_24 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_7*x_24 + x_9*x_24 + x_12*x_24 + x_14*x_24 + x_15*x_24 + x_18*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_23*x_24 + x_1*x_25\n"
                       "        + x_2*x_25 + x_3*x_25 + x_5*x_25 + x_6*x_25 + x_9*x_25 + x_10*x_25 + x_12*x_25 + x_17*x_25 + x_19*x_25 + x_22*x_25 + x_1*x_26 + x_2*x_26 + x_5*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + \n"
                       "        x_10*x_26 + x_12*x_26 + x_13*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_18*x_26 + x_19*x_26 + x_23*x_26 + x_25*x_26 + x_1*x_27 + x_3*x_27 + x_4*x_27 + x_7*x_27 + x_8*x_27 + x_11*x_27 + x_12*x_27\n"
                       "        + x_14*x_27 + x_16*x_27 + x_17*x_27 + x_18*x_27 + x_22*x_27 + x_23*x_27 + x_26*x_27 + x_3*x_28 + x_4*x_28 + x_6*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + \n"
                       "        x_15*x_28 + x_20*x_28 + x_21*x_28 + x_26*x_28 + x_27*x_28 + x_3*x_29 + x_7*x_29 + x_8*x_29 + x_13*x_29 + x_14*x_29 + x_18*x_29 + x_19*x_29 + x_23*x_29 + x_24*x_29 + x_27*x_29 + x_1*x_30 + x_2*x_30\n"
                       "        + x_3*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_13*x_30 + x_15*x_30 + x_19*x_30 + x_20*x_30 + x_21*x_30 + x_22*x_30 + x_25*x_30 + x_28*x_30 + x_1*x_31 + x_2*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 +\n"
                       "        x_8*x_31 + x_11*x_31 + x_13*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_19*x_31 + x_20*x_31 + x_21*x_31 + x_23*x_31 + x_27*x_31 + x_28*x_31 + x_30*x_31 + x_2*x_32 + x_4*x_32 + \n"
                       "        x_5*x_32 + x_7*x_32 + x_8*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_18*x_32 + x_24*x_32 + x_25*x_32 + x_6*x_33 + x_10*x_33 + x_11*x_33 + x_12*x_33 + x_14*x_33 + x_16*x_33 + \n"
                       "        x_20*x_33 + x_21*x_33 + x_24*x_33 + x_26*x_33 + x_30*x_33 + x_3*x_34 + x_7*x_34 + x_10*x_34 + x_11*x_34 + x_13*x_34 + x_15*x_34 + x_16*x_34 + x_17*x_34 + x_19*x_34 + x_20*x_34 + x_21*x_34 + \n"
                       "        x_22*x_34 + x_24*x_34 + x_29*x_34 + x_31*x_34 + x_33*x_34 + x_1*x_35 + x_2*x_35 + x_3*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_14*x_35 + x_15*x_35 + x_21*x_35 +\n"
                       "        x_22*x_35 + x_23*x_35 + x_25*x_35 + x_26*x_35 + x_27*x_35 + x_32*x_35 + x_34*x_35 + x_1*x_36 + x_4*x_36 + x_5*x_36 + x_7*x_36 + x_9*x_36 + x_10*x_36 + x_12*x_36 + x_14*x_36 + x_17*x_36 + x_19*x_36\n"
                       "        + x_20*x_36 + x_21*x_36 + x_22*x_36 + x_24*x_36 + x_25*x_36 + x_28*x_36 + x_29*x_36 + x_30*x_36 + x_31*x_36 + x_32*x_36 + x_33*x_36 + x_34*x_36 + x_2*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + \n"
                       "        x_10*x_37 + x_12*x_37 + x_15*x_37 + x_17*x_37 + x_18*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + x_5*x_38 + x_11*x_38 + x_12*x_38 + x_18*x_38 + x_20*x_38 + \n"
                       "        x_26*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_35*x_38 + x_36*x_38 + x_4*x_39 + x_7*x_39 + x_8*x_39 + x_9*x_39 + x_10*x_39 + x_12*x_39 + x_16*x_39 + x_17*x_39 + \n"
                       "        x_18*x_39 + x_20*x_39 + x_23*x_39 + x_29*x_39 + x_31*x_39 + x_35*x_39 + x_36*x_39 + x_6*x_40 + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_15*x_40 + x_17*x_40 + x_19*x_40 + x_20*x_40 + x_25*x_40 + \n"
                       "        x_29*x_40 + x_30*x_40 + x_32*x_40 + x_36*x_40 + x_37*x_40 + x_38*x_40 + x_39*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_4*x_41 + x_5*x_41 + x_7*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_21*x_41 \n"
                       "        + x_24*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + x_30*x_41 + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_37*x_41 + x_40*x_41 + x_1*x_42 + x_4*x_42 + x_5*x_42 + x_7*x_42 + \n"
                       "        x_8*x_42 + x_9*x_42 + x_10*x_42 + x_13*x_42 + x_14*x_42 + x_16*x_42 + x_17*x_42 + x_19*x_42 + x_22*x_42 + x_24*x_42 + x_26*x_42 + x_28*x_42 + x_29*x_42 + x_34*x_42 + x_35*x_42 + x_36*x_42 + \n"
                       "        x_37*x_42 + x_39*x_42 + x_41*x_42 + x_1*x_43 + x_4*x_43 + x_5*x_43 + x_6*x_43 + x_8*x_43 + x_10*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_19*x_43 + x_20*x_43 + x_21*x_43 + x_22*x_43 + x_23*x_43\n"
                       "        + x_26*x_43 + x_28*x_43 + x_29*x_43 + x_30*x_43 + x_31*x_43 + x_32*x_43 + x_33*x_43 + x_34*x_43 + x_35*x_43 + x_37*x_43 + x_38*x_43 + x_40*x_43 + x_1*x_44 + x_4*x_44 + x_7*x_44 + x_8*x_44 + \n"
                       "        x_9*x_44 + x_10*x_44 + x_11*x_44 + x_14*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_19*x_44 + x_20*x_44 + x_21*x_44 + x_22*x_44 + x_26*x_44 + x_28*x_44 + x_30*x_44 + x_31*x_44 + x_32*x_44 + \n"
                       "        x_35*x_44 + x_36*x_44 + x_37*x_44 + x_39*x_44 + x_40*x_44 + x_43*x_44 + x_2*x_45 + x_3*x_45 + x_4*x_45 + x_6*x_45 + x_9*x_45 + x_10*x_45 + x_13*x_45 + x_16*x_45 + x_17*x_45 + x_19*x_45 + x_22*x_45\n"
                       "        + x_23*x_45 + x_24*x_45 + x_25*x_45 + x_32*x_45 + x_34*x_45 + x_35*x_45 + x_36*x_45 + x_38*x_45 + x_39*x_45 + x_5*x_46 + x_6*x_46 + x_7*x_46 + x_10*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + \n"
                       "        x_14*x_46 + x_15*x_46 + x_16*x_46 + x_20*x_46 + x_22*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_27*x_46 + x_32*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_38*x_46 + x_40*x_46 + x_44*x_46 + \n"
                       "        x_1*x_47 + x_3*x_47 + x_6*x_47 + x_8*x_47 + x_9*x_47 + x_11*x_47 + x_12*x_47 + x_13*x_47 + x_15*x_47 + x_18*x_47 + x_19*x_47 + x_26*x_47 + x_29*x_47 + x_30*x_47 + x_32*x_47 + x_34*x_47 + x_35*x_47\n"
                       "        + x_40*x_47 + x_42*x_47 + x_1*x_48 + x_2*x_48 + x_3*x_48 + x_8*x_48 + x_9*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_15*x_48 + x_16*x_48 + x_20*x_48 + x_21*x_48 + x_22*x_48 + x_23*x_48 + \n"
                       "        x_24*x_48 + x_25*x_48 + x_27*x_48 + x_28*x_48 + x_29*x_48 + x_31*x_48 + x_32*x_48 + x_34*x_48 + x_36*x_48 + x_37*x_48 + x_40*x_48 + x_43*x_48 + x_45*x_48 + x_46*x_48 + x_1*x_49 + x_6*x_49 + \n"
                       "        x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_14*x_49 + x_15*x_49 + x_20*x_49 + x_22*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_29*x_49 + x_30*x_49 + x_31*x_49 + x_32*x_49 + x_33*x_49 + \n"
                       "        x_39*x_49 + x_40*x_49 + x_41*x_49 + x_44*x_49 + x_4 + x_5 + x_7 + x_14 + x_16 + x_18 + x_19 + x_20 + x_21 + x_23 + x_24 + x_28 + x_31 + x_39 + x_42 + x_44 + x_45 + x_49,\n"
                       "    x_1*x_2 + x_1*x_3 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_1*x_6 + x_2*x_6 + x_4*x_6 + x_5*x_6 + x_2*x_7 + x_4*x_7 + x_5*x_7 + x_6*x_7 + x_1*x_8 + x_2*x_8 + x_6*x_8 + x_3*x_9 + x_7*x_9 + x_7*x_10 + x_9*x_10 \n"
                       "        + x_1*x_11 + x_2*x_11 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_9*x_11 + x_10*x_11 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_1*x_13 + x_2*x_13 + x_4*x_13 + \n"
                       "        x_8*x_13 + x_10*x_13 + x_2*x_14 + x_3*x_14 + x_4*x_14 + x_5*x_14 + x_10*x_14 + x_13*x_14 + x_4*x_15 + x_6*x_15 + x_7*x_15 + x_9*x_15 + x_11*x_15 + x_12*x_15 + x_13*x_15 + x_14*x_15 + x_1*x_16 + \n"
                       "        x_2*x_16 + x_4*x_16 + x_7*x_16 + x_12*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_11*x_17 + x_12*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_7*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + \n"
                       "        x_13*x_18 + x_17*x_18 + x_1*x_19 + x_2*x_19 + x_12*x_19 + x_14*x_19 + x_16*x_19 + x_17*x_19 + x_18*x_19 + x_2*x_20 + x_3*x_20 + x_4*x_20 + x_5*x_20 + x_6*x_20 + x_7*x_20 + x_9*x_20 + x_10*x_20 + \n"
                       "        x_11*x_20 + x_12*x_20 + x_17*x_20 + x_18*x_20 + x_19*x_20 + x_1*x_21 + x_5*x_21 + x_6*x_21 + x_14*x_21 + x_15*x_21 + x_17*x_21 + x_20*x_21 + x_4*x_22 + x_7*x_22 + x_9*x_22 + x_11*x_22 + x_12*x_22 \n"
                       "        + x_13*x_22 + x_18*x_22 + x_20*x_22 + x_1*x_23 + x_2*x_23 + x_3*x_23 + x_6*x_23 + x_7*x_23 + x_8*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_15*x_23 + x_17*x_23 + x_18*x_23 + \n"
                       "        x_19*x_23 + x_20*x_23 + x_21*x_23 + x_22*x_23 + x_1*x_24 + x_3*x_24 + x_5*x_24 + x_6*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_14*x_24 + x_15*x_24 + x_19*x_24 + x_21*x_24 + x_23*x_24 \n"
                       "        + x_1*x_25 + x_2*x_25 + x_8*x_25 + x_9*x_25 + x_11*x_25 + x_13*x_25 + x_14*x_25 + x_15*x_25 + x_16*x_25 + x_17*x_25 + x_18*x_25 + x_19*x_25 + x_20*x_25 + x_22*x_25 + x_1*x_26 + x_9*x_26 + \n"
                       "        x_11*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_16*x_26 + x_17*x_26 + x_19*x_26 + x_20*x_26 + x_25*x_26 + x_2*x_27 + x_4*x_27 + x_9*x_27 + x_10*x_27 + x_17*x_27 + x_18*x_27 + x_20*x_27 + \n"
                       "        x_21*x_27 + x_25*x_27 + x_26*x_27 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_9*x_28 + x_10*x_28 + x_13*x_28 + x_14*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_19*x_28 + x_22*x_28 + x_24*x_28 + \n"
                       "        x_25*x_28 + x_27*x_28 + x_3*x_29 + x_5*x_29 + x_6*x_29 + x_10*x_29 + x_12*x_29 + x_13*x_29 + x_15*x_29 + x_16*x_29 + x_18*x_29 + x_20*x_29 + x_21*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_3*x_30\n"
                       "        + x_8*x_30 + x_10*x_30 + x_12*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_16*x_30 + x_18*x_30 + x_20*x_30 + x_21*x_30 + x_24*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + x_1*x_31 + x_2*x_31 + \n"
                       "        x_3*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_8*x_31 + x_9*x_31 + x_11*x_31 + x_12*x_31 + x_13*x_31 + x_15*x_31 + x_17*x_31 + x_19*x_31 + x_24*x_31 + x_25*x_31 + x_26*x_31 + x_27*x_31 + x_29*x_31 \n"
                       "        + x_30*x_31 + x_1*x_32 + x_2*x_32 + x_6*x_32 + x_7*x_32 + x_13*x_32 + x_16*x_32 + x_17*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_22*x_32 + x_24*x_32 + x_27*x_32 + x_29*x_32 + x_1*x_33 + \n"
                       "        x_3*x_33 + x_6*x_33 + x_9*x_33 + x_10*x_33 + x_11*x_33 + x_13*x_33 + x_18*x_33 + x_19*x_33 + x_21*x_33 + x_22*x_33 + x_23*x_33 + x_28*x_33 + x_30*x_33 + x_31*x_33 + x_32*x_33 + x_1*x_34 + x_4*x_34\n"
                       "        + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_16*x_34 + x_17*x_34 + x_18*x_34 + x_21*x_34 + x_22*x_34 + x_26*x_34 + x_30*x_34 + x_33*x_34 + x_1*x_35 + \n"
                       "        x_3*x_35 + x_6*x_35 + x_8*x_35 + x_16*x_35 + x_23*x_35 + x_25*x_35 + x_28*x_35 + x_30*x_35 + x_33*x_35 + x_1*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_7*x_36 + x_9*x_36 + x_12*x_36 + x_13*x_36 + \n"
                       "        x_14*x_36 + x_15*x_36 + x_16*x_36 + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_29*x_36 + x_31*x_36 + x_34*x_36 + x_1*x_37 + x_2*x_37 + x_5*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 +\n"
                       "        x_11*x_37 + x_12*x_37 + x_14*x_37 + x_16*x_37 + x_18*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 + x_22*x_37 + x_25*x_37 + x_27*x_37 + x_32*x_37 + x_35*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + \n"
                       "        x_6*x_38 + x_7*x_38 + x_8*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_13*x_38 + x_16*x_38 + x_18*x_38 + x_19*x_38 + x_20*x_38 + x_24*x_38 + x_25*x_38 + x_29*x_38 + x_32*x_38 + x_33*x_38 + \n"
                       "        x_34*x_38 + x_37*x_38 + x_1*x_39 + x_3*x_39 + x_4*x_39 + x_10*x_39 + x_11*x_39 + x_12*x_39 + x_13*x_39 + x_15*x_39 + x_16*x_39 + x_19*x_39 + x_20*x_39 + x_21*x_39 + x_23*x_39 + x_30*x_39 + \n"
                       "        x_31*x_39 + x_34*x_39 + x_35*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_4*x_40 + x_8*x_40 + x_10*x_40 + x_12*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_16*x_40 + x_20*x_40 + \n"
                       "        x_21*x_40 + x_23*x_40 + x_25*x_40 + x_28*x_40 + x_29*x_40 + x_32*x_40 + x_33*x_40 + x_34*x_40 + x_35*x_40 + x_36*x_40 + x_37*x_40 + x_39*x_40 + x_1*x_41 + x_5*x_41 + x_6*x_41 + x_9*x_41 + \n"
                       "        x_10*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_22*x_41 + x_25*x_41 + x_32*x_41 + x_34*x_41 + x_35*x_41 + x_37*x_41 + x_38*x_41 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_7*x_42 + x_9*x_42\n"
                       "        + x_11*x_42 + x_12*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_20*x_42 + x_21*x_42 + x_22*x_42 + x_26*x_42 + x_27*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_33*x_42 + x_36*x_42 + x_37*x_42 + \n"
                       "        x_38*x_42 + x_39*x_42 + x_40*x_42 + x_41*x_42 + x_1*x_43 + x_3*x_43 + x_4*x_43 + x_5*x_43 + x_7*x_43 + x_8*x_43 + x_10*x_43 + x_13*x_43 + x_15*x_43 + x_23*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 \n"
                       "        + x_27*x_43 + x_28*x_43 + x_30*x_43 + x_31*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_40*x_43 + x_41*x_43 + x_42*x_43 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_6*x_44 + x_7*x_44 + \n"
                       "        x_10*x_44 + x_12*x_44 + x_15*x_44 + x_16*x_44 + x_19*x_44 + x_21*x_44 + x_23*x_44 + x_24*x_44 + x_26*x_44 + x_30*x_44 + x_32*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_39*x_44 + x_40*x_44 + \n"
                       "        x_2*x_45 + x_3*x_45 + x_4*x_45 + x_9*x_45 + x_12*x_45 + x_13*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + x_19*x_45 + x_22*x_45 + x_23*x_45 + x_27*x_45 + x_30*x_45 + x_32*x_45 + x_36*x_45 + \n"
                       "        x_37*x_45 + x_40*x_45 + x_41*x_45 + x_44*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_5*x_46 + x_6*x_46 + x_8*x_46 + x_11*x_46 + x_14*x_46 + x_15*x_46 + x_16*x_46 + x_17*x_46 + x_20*x_46 + x_21*x_46 \n"
                       "        + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_29*x_46 + x_30*x_46 + x_33*x_46 + x_34*x_46 + x_39*x_46 + x_41*x_46 + x_43*x_46 + x_44*x_46 + x_1*x_47 + x_2*x_47 + x_7*x_47 + x_9*x_47 + x_13*x_47 + \n"
                       "        x_16*x_47 + x_18*x_47 + x_20*x_47 + x_21*x_47 + x_23*x_47 + x_24*x_47 + x_27*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + x_46*x_47 + \n"
                       "        x_2*x_48 + x_6*x_48 + x_8*x_48 + x_10*x_48 + x_11*x_48 + x_13*x_48 + x_15*x_48 + x_16*x_48 + x_18*x_48 + x_19*x_48 + x_20*x_48 + x_22*x_48 + x_23*x_48 + x_27*x_48 + x_28*x_48 + x_31*x_48 + \n"
                       "        x_32*x_48 + x_40*x_48 + x_44*x_48 + x_46*x_48 + x_1*x_49 + x_2*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_18*x_49 + x_24*x_49 + x_27*x_49 + x_30*x_49\n"
                       "        + x_31*x_49 + x_32*x_49 + x_41*x_49 + x_43*x_49 + x_45*x_49 + x_1 + x_2 + x_3 + x_6 + x_9 + x_12 + x_13 + x_19 + x_20 + x_24 + x_25 + x_26 + x_30 + x_31 + x_36 + x_37 + x_39 + x_40 + x_41 + x_45 +\n"
                       "        x_46,\n"
                       "    x_2*x_3 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_2*x_6 + x_1*x_7 + x_5*x_7 + x_1*x_8 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_8*x_9 + x_1*x_10 + x_3*x_10 \n"
                       "        + x_4*x_10 + x_5*x_10 + x_3*x_11 + x_7*x_11 + x_10*x_11 + x_1*x_12 + x_4*x_12 + x_5*x_12 + x_9*x_12 + x_10*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_5*x_13 + x_6*x_13 + x_8*x_13 + x_11*x_13 + \n"
                       "        x_12*x_13 + x_1*x_14 + x_6*x_14 + x_11*x_14 + x_2*x_15 + x_4*x_15 + x_7*x_15 + x_8*x_15 + x_9*x_15 + x_10*x_15 + x_1*x_16 + x_2*x_16 + x_3*x_16 + x_5*x_16 + x_8*x_16 + x_12*x_16 + x_13*x_16 + \n"
                       "        x_14*x_16 + x_15*x_16 + x_1*x_17 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_9*x_17 + x_11*x_17 + x_12*x_17 + x_15*x_17 + x_16*x_17 + x_3*x_18 + x_4*x_18 + x_7*x_18 + x_10*x_18 + x_11*x_18 + \n"
                       "        x_12*x_18 + x_16*x_18 + x_1*x_19 + x_3*x_19 + x_6*x_19 + x_7*x_19 + x_14*x_19 + x_16*x_19 + x_17*x_19 + x_18*x_19 + x_1*x_20 + x_2*x_20 + x_4*x_20 + x_5*x_20 + x_7*x_20 + x_8*x_20 + x_9*x_20 + \n"
                       "        x_10*x_20 + x_11*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20 + x_19*x_20 + x_4*x_21 + x_6*x_21 + x_9*x_21 + x_10*x_21 + x_14*x_21 + x_17*x_21 + x_20*x_21 + x_1*x_22 + x_4*x_22 + x_5*x_22 \n"
                       "        + x_8*x_22 + x_10*x_22 + x_12*x_22 + x_13*x_22 + x_14*x_22 + x_17*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_4*x_23 + x_5*x_23 + x_6*x_23 + x_7*x_23 + x_8*x_23 + x_9*x_23 + x_11*x_23 + x_12*x_23\n"
                       "        + x_14*x_23 + x_15*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23 + x_1*x_24 + x_3*x_24 + x_5*x_24 + x_6*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_15*x_24 + x_20*x_24 + x_2*x_25 + x_4*x_25 \n"
                       "        + x_7*x_25 + x_8*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 + x_13*x_25 + x_14*x_25 + x_15*x_25 + x_17*x_25 + x_18*x_25 + x_19*x_25 + x_20*x_25 + x_23*x_25 + x_24*x_25 + x_1*x_26 + x_3*x_26 + \n"
                       "        x_6*x_26 + x_9*x_26 + x_12*x_26 + x_13*x_26 + x_17*x_26 + x_19*x_26 + x_20*x_26 + x_23*x_26 + x_24*x_26 + x_1*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_15*x_27 + x_16*x_27 + x_20*x_27 + x_21*x_27 \n"
                       "        + x_24*x_27 + x_26*x_27 + x_2*x_28 + x_6*x_28 + x_7*x_28 + x_8*x_28 + x_9*x_28 + x_12*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_21*x_28 + x_22*x_28 + x_23*x_28 + x_26*x_28 + \n"
                       "        x_2*x_29 + x_3*x_29 + x_5*x_29 + x_7*x_29 + x_10*x_29 + x_11*x_29 + x_12*x_29 + x_13*x_29 + x_15*x_29 + x_18*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_26*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 \n"
                       "        + x_4*x_30 + x_5*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_11*x_30 + x_12*x_30 + x_14*x_30 + x_16*x_30 + x_18*x_30 + x_19*x_30 + x_21*x_30 + x_22*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + \n"
                       "        x_2*x_31 + x_3*x_31 + x_5*x_31 + x_6*x_31 + x_8*x_31 + x_11*x_31 + x_12*x_31 + x_13*x_31 + x_14*x_31 + x_16*x_31 + x_17*x_31 + x_19*x_31 + x_20*x_31 + x_21*x_31 + x_23*x_31 + x_24*x_31 + x_25*x_31\n"
                       "        + x_27*x_31 + x_2*x_32 + x_3*x_32 + x_4*x_32 + x_5*x_32 + x_6*x_32 + x_8*x_32 + x_10*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_17*x_32 + x_21*x_32 + x_22*x_32 + x_24*x_32 + \n"
                       "        x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_31*x_32 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_12*x_33 + x_13*x_33 + x_14*x_33 + x_15*x_33 + x_16*x_33 + x_19*x_33 + x_20*x_33 + x_23*x_33 + \n"
                       "        x_25*x_33 + x_26*x_33 + x_27*x_33 + x_28*x_33 + x_29*x_33 + x_31*x_33 + x_32*x_33 + x_1*x_34 + x_2*x_34 + x_4*x_34 + x_5*x_34 + x_8*x_34 + x_10*x_34 + x_12*x_34 + x_13*x_34 + x_15*x_34 + x_19*x_34\n"
                       "        + x_22*x_34 + x_27*x_34 + x_28*x_34 + x_29*x_34 + x_30*x_34 + x_31*x_34 + x_2*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_11*x_35 + x_13*x_35 + x_15*x_35 + x_16*x_35 + x_18*x_35 + x_23*x_35 + \n"
                       "        x_25*x_35 + x_26*x_35 + x_30*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_2*x_36 + x_3*x_36 + x_6*x_36 + x_8*x_36 + x_11*x_36 + x_12*x_36 + x_14*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + \n"
                       "        x_19*x_36 + x_21*x_36 + x_24*x_36 + x_25*x_36 + x_27*x_36 + x_28*x_36 + x_33*x_36 + x_34*x_36 + x_5*x_37 + x_9*x_37 + x_11*x_37 + x_12*x_37 + x_14*x_37 + x_15*x_37 + x_16*x_37 + x_17*x_37 + \n"
                       "        x_18*x_37 + x_21*x_37 + x_22*x_37 + x_23*x_37 + x_25*x_37 + x_26*x_37 + x_27*x_37 + x_30*x_37 + x_32*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_9*x_38 + x_10*x_38\n"
                       "        + x_14*x_38 + x_16*x_38 + x_17*x_38 + x_27*x_38 + x_30*x_38 + x_32*x_38 + x_33*x_38 + x_35*x_38 + x_37*x_38 + x_2*x_39 + x_4*x_39 + x_6*x_39 + x_10*x_39 + x_11*x_39 + x_14*x_39 + x_18*x_39 + \n"
                       "        x_22*x_39 + x_23*x_39 + x_24*x_39 + x_26*x_39 + x_30*x_39 + x_32*x_39 + x_34*x_39 + x_35*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_4*x_40 + x_5*x_40 + x_6*x_40 + x_9*x_40 + x_10*x_40 \n"
                       "        + x_11*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_29*x_40 + x_30*x_40 + x_31*x_40 + x_33*x_40 + x_35*x_40 + x_36*x_40 + \n"
                       "        x_38*x_40 + x_39*x_40 + x_1*x_41 + x_3*x_41 + x_4*x_41 + x_5*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_18*x_41 + x_19*x_41 + x_21*x_41 + x_25*x_41 +\n"
                       "        x_31*x_41 + x_32*x_41 + x_35*x_41 + x_36*x_41 + x_37*x_41 + x_38*x_41 + x_39*x_41 + x_40*x_41 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_7*x_42 + x_8*x_42 + x_9*x_42 + x_12*x_42 + x_13*x_42 + x_14*x_42 \n"
                       "        + x_23*x_42 + x_24*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_36*x_42 + x_37*x_42 + x_39*x_42 + x_41*x_42 + x_3*x_43 + x_4*x_43 + x_5*x_43 + x_7*x_43 + \n"
                       "        x_8*x_43 + x_9*x_43 + x_11*x_43 + x_13*x_43 + x_14*x_43 + x_17*x_43 + x_18*x_43 + x_19*x_43 + x_20*x_43 + x_21*x_43 + x_23*x_43 + x_25*x_43 + x_29*x_43 + x_30*x_43 + x_31*x_43 + x_32*x_43 + \n"
                       "        x_34*x_43 + x_35*x_43 + x_37*x_43 + x_38*x_43 + x_1*x_44 + x_4*x_44 + x_6*x_44 + x_8*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_17*x_44 + x_18*x_44 + x_20*x_44 + x_21*x_44 + \n"
                       "        x_25*x_44 + x_29*x_44 + x_30*x_44 + x_33*x_44 + x_35*x_44 + x_36*x_44 + x_38*x_44 + x_39*x_44 + x_40*x_44 + x_42*x_44 + x_2*x_45 + x_4*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_10*x_45 \n"
                       "        + x_12*x_45 + x_13*x_45 + x_19*x_45 + x_21*x_45 + x_25*x_45 + x_28*x_45 + x_30*x_45 + x_31*x_45 + x_34*x_45 + x_35*x_45 + x_40*x_45 + x_2*x_46 + x_3*x_46 + x_5*x_46 + x_6*x_46 + x_8*x_46 + \n"
                       "        x_11*x_46 + x_13*x_46 + x_17*x_46 + x_18*x_46 + x_20*x_46 + x_24*x_46 + x_26*x_46 + x_27*x_46 + x_28*x_46 + x_29*x_46 + x_31*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_37*x_46 + \n"
                       "        x_39*x_46 + x_40*x_46 + x_44*x_46 + x_45*x_46 + x_2*x_47 + x_3*x_47 + x_4*x_47 + x_5*x_47 + x_6*x_47 + x_7*x_47 + x_10*x_47 + x_11*x_47 + x_13*x_47 + x_16*x_47 + x_18*x_47 + x_19*x_47 + x_20*x_47 \n"
                       "        + x_23*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_37*x_47 + x_38*x_47 + x_40*x_47 + x_42*x_47 + x_44*x_47 + x_45*x_47 + x_3*x_48 + x_4*x_48 + x_7*x_48 + \n"
                       "        x_9*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_15*x_48 + x_20*x_48 + x_22*x_48 + x_25*x_48 + x_26*x_48 + x_27*x_48 + x_32*x_48 + x_33*x_48 + x_34*x_48 + x_36*x_48 + x_37*x_48 + x_39*x_48 + \n"
                       "        x_40*x_48 + x_43*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_1*x_49 + x_2*x_49 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_11*x_49 + x_12*x_49 + x_16*x_49 + x_21*x_49 +\n"
                       "        x_22*x_49 + x_24*x_49 + x_28*x_49 + x_30*x_49 + x_31*x_49 + x_32*x_49 + x_34*x_49 + x_37*x_49 + x_38*x_49 + x_40*x_49 + x_42*x_49 + x_44*x_49 + x_46*x_49 + x_2 + x_3 + x_5 + x_7 + x_8 + x_9 + x_10\n"
                       "        + x_11 + x_14 + x_15 + x_16 + x_18 + x_20 + x_21 + x_22 + x_23 + x_24 + x_28 + x_29 + x_31 + x_32 + x_36 + x_37 + x_39 + x_40 + x_41 + x_42 + x_44 + x_47 + x_48 + x_49,\n"
                       "    x_1*x_2 + x_3*x_4 + x_3*x_5 + x_4*x_5 + x_3*x_6 + x_4*x_7 + x_2*x_8 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_1*x_10 + x_2*x_10 + x_3*x_10 + x_4*x_10 + x_6*x_10 + x_9*x_10 + x_1*x_11 + \n"
                       "        x_2*x_11 + x_3*x_11 + x_7*x_11 + x_8*x_11 + x_9*x_11 + x_1*x_12 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_3*x_13 + \n"
                       "        x_4*x_13 + x_6*x_13 + x_8*x_13 + x_10*x_13 + x_12*x_13 + x_1*x_14 + x_3*x_14 + x_5*x_14 + x_12*x_14 + x_13*x_14 + x_1*x_15 + x_3*x_15 + x_5*x_15 + x_6*x_15 + x_9*x_15 + x_10*x_15 + x_13*x_15 + \n"
                       "        x_14*x_15 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_6*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_7*x_17 + x_8*x_17 + \n"
                       "        x_9*x_17 + x_10*x_17 + x_11*x_17 + x_14*x_17 + x_15*x_17 + x_1*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_9*x_18 + x_10*x_18 + x_13*x_18 + x_17*x_18 + x_1*x_19 + x_3*x_19 + \n"
                       "        x_5*x_19 + x_8*x_19 + x_9*x_19 + x_15*x_19 + x_17*x_19 + x_18*x_19 + x_1*x_20 + x_3*x_20 + x_5*x_20 + x_7*x_20 + x_10*x_20 + x_11*x_20 + x_12*x_20 + x_13*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 +\n"
                       "        x_2*x_21 + x_6*x_21 + x_7*x_21 + x_9*x_21 + x_11*x_21 + x_13*x_21 + x_15*x_21 + x_16*x_21 + x_17*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_2*x_22 + x_6*x_22 + x_7*x_22 + x_8*x_22 + x_12*x_22 + \n"
                       "        x_17*x_22 + x_20*x_22 + x_21*x_22 + x_2*x_23 + x_3*x_23 + x_6*x_23 + x_8*x_23 + x_11*x_23 + x_13*x_23 + x_14*x_23 + x_15*x_23 + x_16*x_23 + x_18*x_23 + x_19*x_23 + x_2*x_24 + x_3*x_24 + x_4*x_24 +\n"
                       "        x_6*x_24 + x_8*x_24 + x_13*x_24 + x_14*x_24 + x_16*x_24 + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_23*x_24 + x_1*x_25 + x_5*x_25 + x_6*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 +\n"
                       "        x_13*x_25 + x_14*x_25 + x_17*x_25 + x_18*x_25 + x_19*x_25 + x_21*x_25 + x_24*x_25 + x_2*x_26 + x_5*x_26 + x_7*x_26 + x_13*x_26 + x_16*x_26 + x_17*x_26 + x_18*x_26 + x_20*x_26 + x_22*x_26 + \n"
                       "        x_23*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_10*x_27 + x_11*x_27 + x_14*x_27 + x_15*x_27 + x_17*x_27 + x_18*x_27 + x_19*x_27 + x_22*x_27 + x_1*x_28 +\n"
                       "        x_3*x_28 + x_5*x_28 + x_7*x_28 + x_9*x_28 + x_13*x_28 + x_14*x_28 + x_15*x_28 + x_17*x_28 + x_19*x_28 + x_20*x_28 + x_21*x_28 + x_23*x_28 + x_24*x_28 + x_26*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 +\n"
                       "        x_4*x_29 + x_5*x_29 + x_6*x_29 + x_7*x_29 + x_12*x_29 + x_13*x_29 + x_14*x_29 + x_16*x_29 + x_17*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_24*x_29 + x_25*x_29 + x_26*x_29 + x_27*x_29 + \n"
                       "        x_28*x_29 + x_2*x_30 + x_4*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_11*x_30 + x_12*x_30 + x_14*x_30 + x_15*x_30 + x_16*x_30 + x_17*x_30 + x_18*x_30 + x_21*x_30 + x_23*x_30 + x_24*x_30 \n"
                       "        + x_25*x_30 + x_26*x_30 + x_28*x_30 + x_4*x_31 + x_8*x_31 + x_9*x_31 + x_10*x_31 + x_12*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_19*x_31 + x_21*x_31 + x_22*x_31 + \n"
                       "        x_25*x_31 + x_26*x_31 + x_27*x_31 + x_28*x_31 + x_30*x_31 + x_1*x_32 + x_4*x_32 + x_5*x_32 + x_6*x_32 + x_8*x_32 + x_9*x_32 + x_10*x_32 + x_11*x_32 + x_16*x_32 + x_17*x_32 + x_19*x_32 + x_20*x_32 \n"
                       "        + x_21*x_32 + x_26*x_32 + x_31*x_32 + x_2*x_33 + x_5*x_33 + x_7*x_33 + x_9*x_33 + x_11*x_33 + x_13*x_33 + x_14*x_33 + x_18*x_33 + x_20*x_33 + x_22*x_33 + x_24*x_33 + x_26*x_33 + x_27*x_33 + \n"
                       "        x_32*x_33 + x_1*x_34 + x_2*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 + x_9*x_34 + x_10*x_34 + x_11*x_34 + x_14*x_34 + x_16*x_34 + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_21*x_34 + x_24*x_34 +\n"
                       "        x_25*x_34 + x_26*x_34 + x_27*x_34 + x_28*x_34 + x_32*x_34 + x_1*x_35 + x_3*x_35 + x_6*x_35 + x_7*x_35 + x_8*x_35 + x_12*x_35 + x_13*x_35 + x_16*x_35 + x_18*x_35 + x_19*x_35 + x_20*x_35 + x_21*x_35\n"
                       "        + x_22*x_35 + x_23*x_35 + x_28*x_35 + x_29*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_4*x_36 + x_6*x_36 + x_7*x_36 + x_10*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_14*x_36 + x_15*x_36 + \n"
                       "        x_19*x_36 + x_21*x_36 + x_22*x_36 + x_25*x_36 + x_26*x_36 + x_29*x_36 + x_30*x_36 + x_31*x_36 + x_33*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37\n"
                       "        + x_12*x_37 + x_17*x_37 + x_18*x_37 + x_19*x_37 + x_22*x_37 + x_25*x_37 + x_26*x_37 + x_27*x_37 + x_28*x_37 + x_30*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + \n"
                       "        x_5*x_38 + x_6*x_38 + x_9*x_38 + x_11*x_38 + x_17*x_38 + x_18*x_38 + x_20*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + x_27*x_38 + x_29*x_38 + x_32*x_38 + x_34*x_38 + x_35*x_38 + x_36*x_38 + \n"
                       "        x_3*x_39 + x_5*x_39 + x_8*x_39 + x_10*x_39 + x_11*x_39 + x_13*x_39 + x_14*x_39 + x_17*x_39 + x_18*x_39 + x_19*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + \n"
                       "        x_27*x_39 + x_28*x_39 + x_29*x_39 + x_35*x_39 + x_36*x_39 + x_2*x_40 + x_3*x_40 + x_4*x_40 + x_6*x_40 + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_13*x_40 + x_14*x_40 + x_16*x_40 + x_18*x_40 + x_22*x_40 \n"
                       "        + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_28*x_40 + x_29*x_40 + x_31*x_40 + x_36*x_40 + x_38*x_40 + x_1*x_41 + x_2*x_41 + x_4*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_10*x_41 + x_11*x_41 + \n"
                       "        x_13*x_41 + x_16*x_41 + x_19*x_41 + x_20*x_41 + x_23*x_41 + x_25*x_41 + x_26*x_41 + x_27*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_32*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_40*x_41 + \n"
                       "        x_2*x_42 + x_3*x_42 + x_8*x_42 + x_9*x_42 + x_10*x_42 + x_11*x_42 + x_13*x_42 + x_14*x_42 + x_19*x_42 + x_23*x_42 + x_24*x_42 + x_25*x_42 + x_29*x_42 + x_30*x_42 + x_33*x_42 + x_34*x_42 + \n"
                       "        x_37*x_42 + x_39*x_42 + x_40*x_42 + x_1*x_43 + x_2*x_43 + x_6*x_43 + x_7*x_43 + x_10*x_43 + x_12*x_43 + x_13*x_43 + x_15*x_43 + x_16*x_43 + x_18*x_43 + x_19*x_43 + x_20*x_43 + x_23*x_43 + \n"
                       "        x_24*x_43 + x_25*x_43 + x_27*x_43 + x_28*x_43 + x_31*x_43 + x_32*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_38*x_43 + x_2*x_44 + x_7*x_44 + x_10*x_44 + x_12*x_44 + x_13*x_44 + x_15*x_44 + \n"
                       "        x_16*x_44 + x_19*x_44 + x_20*x_44 + x_22*x_44 + x_24*x_44 + x_26*x_44 + x_29*x_44 + x_30*x_44 + x_38*x_44 + x_39*x_44 + x_3*x_45 + x_4*x_45 + x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_9*x_45 +\n"
                       "        x_11*x_45 + x_16*x_45 + x_20*x_45 + x_22*x_45 + x_27*x_45 + x_29*x_45 + x_33*x_45 + x_35*x_45 + x_36*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_42*x_45 + x_44*x_45 + x_2*x_46 + x_3*x_46 + \n"
                       "        x_4*x_46 + x_6*x_46 + x_8*x_46 + x_9*x_46 + x_13*x_46 + x_20*x_46 + x_21*x_46 + x_24*x_46 + x_31*x_46 + x_34*x_46 + x_35*x_46 + x_38*x_46 + x_40*x_46 + x_41*x_46 + x_42*x_46 + x_43*x_46 + \n"
                       "        x_45*x_46 + x_3*x_47 + x_5*x_47 + x_6*x_47 + x_7*x_47 + x_8*x_47 + x_11*x_47 + x_12*x_47 + x_14*x_47 + x_15*x_47 + x_16*x_47 + x_17*x_47 + x_19*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_25*x_47\n"
                       "        + x_26*x_47 + x_27*x_47 + x_28*x_47 + x_30*x_47 + x_31*x_47 + x_37*x_47 + x_39*x_47 + x_40*x_47 + x_42*x_47 + x_43*x_47 + x_45*x_47 + x_46*x_47 + x_4*x_48 + x_6*x_48 + x_10*x_48 + x_11*x_48 + \n"
                       "        x_12*x_48 + x_13*x_48 + x_14*x_48 + x_15*x_48 + x_18*x_48 + x_19*x_48 + x_20*x_48 + x_21*x_48 + x_24*x_48 + x_25*x_48 + x_26*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_34*x_48 + x_41*x_48 + \n"
                       "        x_43*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_4*x_49 + x_5*x_49 + x_6*x_49 + x_8*x_49 + x_9*x_49 + x_12*x_49 + x_15*x_49 + x_17*x_49 + x_20*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49\n"
                       "        + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_30*x_49 + x_31*x_49 + x_32*x_49 + x_35*x_49 + x_36*x_49 + x_37*x_49 + x_39*x_49 + x_41*x_49 + x_42*x_49 + x_43*x_49 + x_45*x_49 + x_46*x_49 + x_2 + x_3 + \n"
                       "        x_4 + x_5 + x_10 + x_14 + x_15 + x_16 + x_18 + x_19 + x_23 + x_28 + x_29 + x_31 + x_35 + x_37 + x_39 + x_40 + x_43 + x_45,\n"
                       "    x_1*x_3 + x_2*x_3 + x_1*x_4 + x_3*x_5 + x_4*x_5 + x_1*x_7 + x_2*x_7 + x_6*x_7 + x_1*x_8 + x_2*x_8 + x_3*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_7*x_9 + x_8*x_9 + x_1*x_10 + x_1*x_11 \n"
                       "        + x_2*x_11 + x_8*x_11 + x_10*x_11 + x_4*x_12 + x_6*x_12 + x_8*x_12 + x_9*x_12 + x_10*x_12 + x_2*x_13 + x_3*x_13 + x_5*x_13 + x_11*x_13 + x_1*x_14 + x_3*x_14 + x_7*x_14 + x_8*x_14 + x_10*x_14 + \n"
                       "        x_11*x_14 + x_12*x_14 + x_1*x_15 + x_3*x_15 + x_6*x_15 + x_10*x_15 + x_14*x_15 + x_2*x_16 + x_3*x_16 + x_4*x_16 + x_12*x_16 + x_13*x_16 + x_1*x_17 + x_2*x_17 + x_6*x_17 + x_8*x_17 + x_10*x_17 + \n"
                       "        x_11*x_17 + x_12*x_17 + x_13*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_7*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_13*x_18 + x_16*x_18 + x_2*x_19 + x_3*x_19 + x_6*x_19 + x_10*x_19 + \n"
                       "        x_11*x_19 + x_14*x_19 + x_15*x_19 + x_17*x_19 + x_1*x_20 + x_2*x_20 + x_4*x_20 + x_5*x_20 + x_11*x_20 + x_12*x_20 + x_14*x_20 + x_15*x_20 + x_17*x_20 + x_18*x_20 + x_2*x_21 + x_8*x_21 + x_10*x_21 \n"
                       "        + x_11*x_21 + x_12*x_21 + x_13*x_21 + x_15*x_21 + x_16*x_21 + x_19*x_21 + x_2*x_22 + x_4*x_22 + x_5*x_22 + x_6*x_22 + x_9*x_22 + x_10*x_22 + x_12*x_22 + x_13*x_22 + x_14*x_22 + x_15*x_22 + \n"
                       "        x_16*x_22 + x_17*x_22 + x_18*x_22 + x_20*x_22 + x_1*x_23 + x_3*x_23 + x_4*x_23 + x_9*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_15*x_23 + x_17*x_23 + x_20*x_23 + x_21*x_23 + x_1*x_24 + x_2*x_24 \n"
                       "        + x_3*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_14*x_24 + x_15*x_24 + x_20*x_24 + x_21*x_24 + x_1*x_25 + x_3*x_25 + x_5*x_25 + x_7*x_25 + x_10*x_25 + \n"
                       "        x_12*x_25 + x_13*x_25 + x_14*x_25 + x_16*x_25 + x_17*x_25 + x_19*x_25 + x_1*x_26 + x_3*x_26 + x_5*x_26 + x_8*x_26 + x_10*x_26 + x_11*x_26 + x_12*x_26 + x_14*x_26 + x_16*x_26 + x_20*x_26 + \n"
                       "        x_23*x_26 + x_24*x_26 + x_2*x_27 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_9*x_27 + x_12*x_27 + x_14*x_27 + x_16*x_27 + x_17*x_27 + x_18*x_27 + x_21*x_27 + x_22*x_27 + x_24*x_27 + x_25*x_27 + x_26*x_27\n"
                       "        + x_1*x_28 + x_3*x_28 + x_4*x_28 + x_5*x_28 + x_10*x_28 + x_16*x_28 + x_21*x_28 + x_22*x_28 + x_25*x_28 + x_27*x_28 + x_3*x_29 + x_7*x_29 + x_8*x_29 + x_10*x_29 + x_16*x_29 + x_17*x_29 + x_18*x_29\n"
                       "        + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_23*x_29 + x_24*x_29 + x_25*x_29 + x_27*x_29 + x_28*x_29 + x_3*x_30 + x_4*x_30 + x_8*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + x_14*x_30 + \n"
                       "        x_15*x_30 + x_19*x_30 + x_21*x_30 + x_22*x_30 + x_23*x_30 + x_24*x_30 + x_26*x_30 + x_28*x_30 + x_1*x_31 + x_4*x_31 + x_6*x_31 + x_8*x_31 + x_9*x_31 + x_11*x_31 + x_13*x_31 + x_14*x_31 + x_17*x_31\n"
                       "        + x_19*x_31 + x_20*x_31 + x_22*x_31 + x_24*x_31 + x_26*x_31 + x_27*x_31 + x_28*x_31 + x_30*x_31 + x_3*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 + \n"
                       "        x_21*x_32 + x_22*x_32 + x_23*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_1*x_33 + x_2*x_33 + x_5*x_33 + x_6*x_33 + x_8*x_33 + x_9*x_33 + x_11*x_33 + x_12*x_33 + x_13*x_33 + x_16*x_33 \n"
                       "        + x_20*x_33 + x_22*x_33 + x_23*x_33 + x_28*x_33 + x_29*x_33 + x_31*x_33 + x_3*x_34 + x_4*x_34 + x_5*x_34 + x_7*x_34 + x_9*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_15*x_34 + x_16*x_34 + \n"
                       "        x_18*x_34 + x_20*x_34 + x_22*x_34 + x_27*x_34 + x_30*x_34 + x_32*x_34 + x_1*x_35 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_6*x_35 + x_7*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_14*x_35 + x_15*x_35 +\n"
                       "        x_16*x_35 + x_18*x_35 + x_20*x_35 + x_21*x_35 + x_24*x_35 + x_25*x_35 + x_27*x_35 + x_32*x_35 + x_33*x_35 + x_34*x_35 + x_3*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_14*x_36 + x_19*x_36 + \n"
                       "        x_20*x_36 + x_21*x_36 + x_23*x_36 + x_24*x_36 + x_27*x_36 + x_28*x_36 + x_32*x_36 + x_34*x_36 + x_35*x_36 + x_4*x_37 + x_5*x_37 + x_11*x_37 + x_12*x_37 + x_13*x_37 + x_15*x_37 + x_17*x_37 + \n"
                       "        x_18*x_37 + x_19*x_37 + x_24*x_37 + x_26*x_37 + x_30*x_37 + x_31*x_37 + x_34*x_37 + x_35*x_37 + x_36*x_37 + x_3*x_38 + x_5*x_38 + x_7*x_38 + x_9*x_38 + x_12*x_38 + x_15*x_38 + x_17*x_38 + \n"
                       "        x_24*x_38 + x_26*x_38 + x_28*x_38 + x_29*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_36*x_38 + x_3*x_39 + x_5*x_39 + x_8*x_39 + x_12*x_39 + x_14*x_39 + x_15*x_39 + x_17*x_39 + x_21*x_39 + \n"
                       "        x_22*x_39 + x_25*x_39 + x_27*x_39 + x_28*x_39 + x_31*x_39 + x_33*x_39 + x_35*x_39 + x_36*x_39 + x_1*x_40 + x_3*x_40 + x_4*x_40 + x_6*x_40 + x_7*x_40 + x_11*x_40 + x_16*x_40 + x_21*x_40 + x_23*x_40\n"
                       "        + x_25*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + x_30*x_40 + x_31*x_40 + x_32*x_40 + x_34*x_40 + x_36*x_40 + x_37*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_4*x_41 + x_6*x_41 + x_8*x_41 + \n"
                       "        x_10*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_20*x_41 + x_21*x_41 + x_23*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + x_32*x_41 + x_34*x_41 + x_35*x_41 + x_38*x_41 + x_40*x_41 + x_1*x_42 + \n"
                       "        x_2*x_42 + x_3*x_42 + x_6*x_42 + x_9*x_42 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_13*x_42 + x_15*x_42 + x_19*x_42 + x_21*x_42 + x_22*x_42 + x_30*x_42 + x_32*x_42 + x_35*x_42 + x_2*x_43 + x_3*x_43 \n"
                       "        + x_4*x_43 + x_9*x_43 + x_12*x_43 + x_16*x_43 + x_17*x_43 + x_21*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + x_26*x_43 + x_28*x_43 + x_32*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_1*x_44 + \n"
                       "        x_2*x_44 + x_3*x_44 + x_7*x_44 + x_9*x_44 + x_10*x_44 + x_13*x_44 + x_16*x_44 + x_18*x_44 + x_19*x_44 + x_20*x_44 + x_23*x_44 + x_25*x_44 + x_30*x_44 + x_35*x_44 + x_36*x_44 + x_38*x_44 + \n"
                       "        x_39*x_44 + x_41*x_44 + x_5*x_45 + x_7*x_45 + x_8*x_45 + x_11*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_20*x_45 + x_23*x_45 + x_24*x_45 + x_26*x_45 + x_31*x_45 + x_33*x_45 + x_35*x_45 + \n"
                       "        x_37*x_45 + x_40*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_2*x_46 + x_5*x_46 + x_6*x_46 + x_9*x_46 + x_10*x_46 + x_14*x_46 + x_16*x_46 + x_17*x_46 + x_18*x_46 + x_19*x_46 + x_20*x_46 + \n"
                       "        x_22*x_46 + x_23*x_46 + x_27*x_46 + x_28*x_46 + x_36*x_46 + x_39*x_46 + x_40*x_46 + x_41*x_46 + x_42*x_46 + x_1*x_47 + x_2*x_47 + x_4*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_18*x_47 + x_19*x_47\n"
                       "        + x_20*x_47 + x_21*x_47 + x_23*x_47 + x_25*x_47 + x_26*x_47 + x_27*x_47 + x_34*x_47 + x_35*x_47 + x_38*x_47 + x_40*x_47 + x_41*x_47 + x_43*x_47 + x_2*x_48 + x_3*x_48 + x_5*x_48 + x_7*x_48 + \n"
                       "        x_12*x_48 + x_14*x_48 + x_15*x_48 + x_17*x_48 + x_18*x_48 + x_21*x_48 + x_22*x_48 + x_23*x_48 + x_25*x_48 + x_28*x_48 + x_30*x_48 + x_31*x_48 + x_33*x_48 + x_35*x_48 + x_39*x_48 + x_40*x_48 + \n"
                       "        x_41*x_48 + x_42*x_48 + x_43*x_48 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_13*x_49 + x_14*x_49 + x_16*x_49 + x_17*x_49 + x_18*x_49 + x_19*x_49 + x_20*x_49\n"
                       "        + x_21*x_49 + x_22*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_31*x_49 + x_33*x_49 + x_34*x_49 + x_36*x_49 + x_38*x_49 + x_41*x_49 + x_42*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + \n"
                       "        x_1 + x_4 + x_6 + x_7 + x_8 + x_9 + x_10 + x_14 + x_16 + x_17 + x_19 + x_20 + x_21 + x_25 + x_26 + x_28 + x_30 + x_33 + x_36 + x_37 + x_43 + x_47,\n"
                       "    x_1*x_2 + x_1*x_3 + x_2*x_4 + x_3*x_4 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_4*x_6 + x_5*x_6 + x_2*x_7 + x_5*x_7 + x_1*x_8 + x_3*x_8 + x_5*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_3*x_9 + x_7*x_9 + x_8*x_9 + \n"
                       "        x_1*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_9*x_11 + x_1*x_12 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_1*x_13 + x_3*x_13 + x_4*x_13 + x_5*x_13 + x_7*x_13 + x_9*x_13 \n"
                       "        + x_10*x_13 + x_12*x_13 + x_2*x_14 + x_4*x_14 + x_5*x_14 + x_8*x_14 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_10*x_15 + x_13*x_15 + x_14*x_15 + x_1*x_16 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_10*x_16 + \n"
                       "        x_13*x_16 + x_14*x_16 + x_15*x_16 + x_1*x_17 + x_3*x_17 + x_4*x_17 + x_6*x_17 + x_7*x_17 + x_8*x_17 + x_13*x_17 + x_15*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_5*x_18 + x_7*x_18 + x_10*x_18 + \n"
                       "        x_11*x_18 + x_13*x_18 + x_17*x_18 + x_2*x_19 + x_7*x_19 + x_8*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_18*x_19 + x_2*x_20 + x_3*x_20 + x_4*x_20 + x_6*x_20 +\n"
                       "        x_7*x_20 + x_10*x_20 + x_12*x_20 + x_13*x_20 + x_15*x_20 + x_19*x_20 + x_6*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_11*x_21 + x_12*x_21 + x_18*x_21 + x_2*x_22 + x_5*x_22 + x_11*x_22 + x_13*x_22 \n"
                       "        + x_14*x_22 + x_17*x_22 + x_20*x_22 + x_4*x_23 + x_5*x_23 + x_6*x_23 + x_8*x_23 + x_10*x_23 + x_13*x_23 + x_14*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23 + x_5*x_24 + x_6*x_24 + x_8*x_24 + x_10*x_24\n"
                       "        + x_11*x_24 + x_12*x_24 + x_14*x_24 + x_16*x_24 + x_18*x_24 + x_19*x_24 + x_21*x_24 + x_1*x_25 + x_3*x_25 + x_4*x_25 + x_5*x_25 + x_6*x_25 + x_8*x_25 + x_9*x_25 + x_11*x_25 + x_12*x_25 + x_13*x_25\n"
                       "        + x_14*x_25 + x_16*x_25 + x_19*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_4*x_26 + x_7*x_26 + x_14*x_26 + x_15*x_26 + x_16*x_26 + x_18*x_26 + x_19*x_26 + \n"
                       "        x_20*x_26 + x_21*x_26 + x_22*x_26 + x_24*x_26 + x_25*x_26 + x_2*x_27 + x_3*x_27 + x_4*x_27 + x_5*x_27 + x_6*x_27 + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_10*x_27 + x_16*x_27 + x_18*x_27 + x_19*x_27 + \n"
                       "        x_21*x_27 + x_23*x_27 + x_2*x_28 + x_5*x_28 + x_6*x_28 + x_8*x_28 + x_11*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + x_15*x_28 + x_16*x_28 + x_18*x_28 + x_20*x_28 + x_21*x_28 + x_22*x_28 + \n"
                       "        x_23*x_28 + x_1*x_29 + x_3*x_29 + x_6*x_29 + x_8*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_14*x_29 + x_15*x_29 + x_17*x_29 + x_20*x_29 + x_25*x_29 + x_26*x_29 + x_27*x_29 + x_3*x_30 + x_5*x_30 +\n"
                       "        x_6*x_30 + x_7*x_30 + x_10*x_30 + x_18*x_30 + x_19*x_30 + x_21*x_30 + x_22*x_30 + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_27*x_30 + x_28*x_30 + x_6*x_31 + x_8*x_31 + x_10*x_31 + x_12*x_31 + \n"
                       "        x_14*x_31 + x_19*x_31 + x_20*x_31 + x_21*x_31 + x_23*x_31 + x_24*x_31 + x_26*x_31 + x_27*x_31 + x_8*x_32 + x_9*x_32 + x_10*x_32 + x_13*x_32 + x_14*x_32 + x_18*x_32 + x_20*x_32 + x_21*x_32 + \n"
                       "        x_22*x_32 + x_23*x_32 + x_25*x_32 + x_27*x_32 + x_29*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_4*x_33 + x_5*x_33 + x_6*x_33 + x_7*x_33 + x_9*x_33 + x_11*x_33 + x_16*x_33 + x_17*x_33 + x_19*x_33 \n"
                       "        + x_23*x_33 + x_25*x_33 + x_26*x_33 + x_28*x_33 + x_30*x_33 + x_31*x_33 + x_1*x_34 + x_3*x_34 + x_5*x_34 + x_11*x_34 + x_12*x_34 + x_13*x_34 + x_14*x_34 + x_15*x_34 + x_16*x_34 + x_18*x_34 + \n"
                       "        x_20*x_34 + x_24*x_34 + x_26*x_34 + x_27*x_34 + x_28*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_1*x_35 + x_2*x_35 + x_3*x_35 + x_4*x_35 + x_8*x_35 + x_10*x_35 + x_11*x_35 + x_14*x_35\n"
                       "        + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_19*x_35 + x_21*x_35 + x_22*x_35 + x_23*x_35 + x_27*x_35 + x_28*x_35 + x_30*x_35 + x_31*x_35 + x_2*x_36 + x_3*x_36 + x_5*x_36 + x_6*x_36 + x_8*x_36 + \n"
                       "        x_10*x_36 + x_14*x_36 + x_20*x_36 + x_23*x_36 + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_29*x_36 + x_30*x_36 + x_31*x_36 + x_32*x_36 + x_33*x_36 + x_34*x_36 + x_2*x_37 + \n"
                       "        x_7*x_37 + x_8*x_37 + x_9*x_37 + x_11*x_37 + x_13*x_37 + x_18*x_37 + x_19*x_37 + x_20*x_37 + x_23*x_37 + x_24*x_37 + x_27*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + x_35*x_37 + x_1*x_38 + x_7*x_38\n"
                       "        + x_8*x_38 + x_13*x_38 + x_14*x_38 + x_17*x_38 + x_18*x_38 + x_23*x_38 + x_24*x_38 + x_25*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_31*x_38 + x_33*x_38 + x_2*x_39 + x_3*x_39 + x_4*x_39 + \n"
                       "        x_5*x_39 + x_7*x_39 + x_8*x_39 + x_13*x_39 + x_14*x_39 + x_18*x_39 + x_19*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + x_26*x_39 + x_28*x_39 + x_29*x_39 + x_31*x_39 + x_32*x_39 + x_33*x_39 + \n"
                       "        x_38*x_39 + x_1*x_40 + x_2*x_40 + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_9*x_40 + x_10*x_40 + x_11*x_40 + x_15*x_40 + x_18*x_40 + x_19*x_40 + x_26*x_40 + x_27*x_40 + x_29*x_40 + x_31*x_40 + x_35*x_40 \n"
                       "        + x_36*x_40 + x_37*x_40 + x_39*x_40 + x_1*x_41 + x_2*x_41 + x_4*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_12*x_41 + x_13*x_41 + x_17*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41 + \n"
                       "        x_23*x_41 + x_29*x_41 + x_30*x_41 + x_35*x_41 + x_36*x_41 + x_38*x_41 + x_39*x_41 + x_40*x_41 + x_1*x_42 + x_2*x_42 + x_3*x_42 + x_4*x_42 + x_6*x_42 + x_7*x_42 + x_8*x_42 + x_9*x_42 + x_10*x_42 + \n"
                       "        x_11*x_42 + x_15*x_42 + x_18*x_42 + x_20*x_42 + x_23*x_42 + x_26*x_42 + x_27*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_33*x_42 + x_35*x_42 + x_36*x_42 + x_38*x_42 + x_1*x_43 + \n"
                       "        x_2*x_43 + x_3*x_43 + x_7*x_43 + x_9*x_43 + x_10*x_43 + x_11*x_43 + x_16*x_43 + x_17*x_43 + x_18*x_43 + x_19*x_43 + x_22*x_43 + x_23*x_43 + x_25*x_43 + x_28*x_43 + x_29*x_43 + x_30*x_43 + \n"
                       "        x_31*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_42*x_43 + x_1*x_44 + x_4*x_44 + x_6*x_44 + x_7*x_44 + x_8*x_44 + x_9*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_16*x_44 + x_17*x_44 + x_20*x_44 \n"
                       "        + x_22*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44 + x_30*x_44 + x_34*x_44 + x_36*x_44 + x_38*x_44 + x_39*x_44 + x_40*x_44 + x_42*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + x_3*x_45 + x_4*x_45 + \n"
                       "        x_5*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_9*x_45 + x_11*x_45 + x_13*x_45 + x_14*x_45 + x_19*x_45 + x_21*x_45 + x_22*x_45 + x_24*x_45 + x_25*x_45 + x_29*x_45 + x_31*x_45 + x_34*x_45 + x_35*x_45\n"
                       "        + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_42*x_45 + x_43*x_45 + x_9*x_46 + x_10*x_46 + x_12*x_46 + x_13*x_46 + x_17*x_46 + x_18*x_46 + x_22*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_27*x_46 + \n"
                       "        x_29*x_46 + x_31*x_46 + x_32*x_46 + x_34*x_46 + x_36*x_46 + x_37*x_46 + x_38*x_46 + x_39*x_46 + x_40*x_46 + x_41*x_46 + x_42*x_46 + x_44*x_46 + x_45*x_46 + x_2*x_47 + x_4*x_47 + x_7*x_47 + \n"
                       "        x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + x_15*x_47 + x_16*x_47 + x_17*x_47 + x_18*x_47 + x_19*x_47 + x_20*x_47 + x_24*x_47 + x_28*x_47 + x_30*x_47 + x_33*x_47 + x_35*x_47 + x_44*x_47 + \n"
                       "        x_45*x_47 + x_2*x_48 + x_6*x_48 + x_10*x_48 + x_16*x_48 + x_21*x_48 + x_22*x_48 + x_23*x_48 + x_27*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_36*x_48 + x_40*x_48 + x_41*x_48 + x_42*x_48 + \n"
                       "        x_43*x_48 + x_45*x_48 + x_47*x_48 + x_3*x_49 + x_6*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_12*x_49 + x_13*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49 + x_31*x_49 + x_35*x_49 + x_39*x_49\n"
                       "        + x_40*x_49 + x_43*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_3 + x_4 + x_7 + x_8 + x_9 + x_10 + x_14 + x_17 + x_23 + x_24 + x_25 + x_29 + x_33 + x_34 + x_35 + x_38 + x_39 + x_41 + x_42 + x_44 +\n"
                       "        x_47 + x_48,\n"
                       "    x_2*x_4 + x_2*x_5 + x_3*x_5 + x_1*x_6 + x_5*x_6 + x_1*x_7 + x_4*x_7 + x_6*x_7 + x_1*x_8 + x_2*x_8 + x_3*x_8 + x_4*x_8 + x_7*x_8 + x_2*x_9 + x_3*x_9 + x_1*x_10 + x_7*x_10 + x_1*x_11 + x_2*x_11 + \n"
                       "        x_4*x_11 + x_5*x_11 + x_2*x_12 + x_8*x_12 + x_9*x_12 + x_10*x_12 + x_1*x_13 + x_4*x_13 + x_6*x_13 + x_9*x_13 + x_10*x_13 + x_11*x_13 + x_4*x_14 + x_6*x_14 + x_9*x_14 + x_13*x_14 + x_1*x_15 + \n"
                       "        x_4*x_15 + x_8*x_15 + x_12*x_15 + x_13*x_15 + x_1*x_16 + x_3*x_16 + x_4*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_13*x_16 + x_14*x_16 + x_3*x_17 + x_4*x_17 + x_6*x_17 + \n"
                       "        x_9*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_2*x_18 + x_3*x_18 + x_5*x_18 + x_6*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + x_2*x_19 + x_4*x_19 + \n"
                       "        x_6*x_19 + x_7*x_19 + x_13*x_19 + x_14*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_1*x_20 + x_2*x_20 + x_3*x_20 + x_5*x_20 + x_11*x_20 + x_13*x_20 + x_14*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20 \n"
                       "        + x_19*x_20 + x_1*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_13*x_21 + x_16*x_21 + x_18*x_21 + x_1*x_22 + x_3*x_22 + x_6*x_22 + x_7*x_22 + x_8*x_22 + x_9*x_22 + \n"
                       "        x_11*x_22 + x_12*x_22 + x_15*x_22 + x_17*x_22 + x_3*x_23 + x_6*x_23 + x_7*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_14*x_23 + x_17*x_23 + x_19*x_23 + x_21*x_23 + x_1*x_24 + x_2*x_24\n"
                       "        + x_4*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_12*x_24 + x_17*x_24 + x_2*x_25 + x_3*x_25 + x_4*x_25 + x_7*x_25 + x_9*x_25 + x_10*x_25 + x_15*x_25 + x_17*x_25 + x_18*x_25 + x_21*x_25 \n"
                       "        + x_3*x_26 + x_4*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_13*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_18*x_26 + x_19*x_26 + x_20*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_3*x_27 \n"
                       "        + x_4*x_27 + x_9*x_27 + x_10*x_27 + x_11*x_27 + x_12*x_27 + x_14*x_27 + x_15*x_27 + x_16*x_27 + x_17*x_27 + x_19*x_27 + x_22*x_27 + x_24*x_27 + x_25*x_27 + x_26*x_27 + x_3*x_28 + x_5*x_28 + \n"
                       "        x_6*x_28 + x_10*x_28 + x_11*x_28 + x_13*x_28 + x_16*x_28 + x_17*x_28 + x_19*x_28 + x_23*x_28 + x_3*x_29 + x_5*x_29 + x_10*x_29 + x_12*x_29 + x_15*x_29 + x_18*x_29 + x_19*x_29 + x_20*x_29 + \n"
                       "        x_24*x_29 + x_25*x_29 + x_26*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_3*x_30 + x_7*x_30 + x_9*x_30 + x_12*x_30 + x_14*x_30 + x_15*x_30 + x_21*x_30 + x_24*x_30 + x_28*x_30 + x_2*x_31 + x_4*x_31 \n"
                       "        + x_5*x_31 + x_6*x_31 + x_9*x_31 + x_10*x_31 + x_15*x_31 + x_17*x_31 + x_19*x_31 + x_20*x_31 + x_22*x_31 + x_28*x_31 + x_30*x_31 + x_3*x_32 + x_5*x_32 + x_6*x_32 + x_7*x_32 + x_11*x_32 + x_12*x_32\n"
                       "        + x_14*x_32 + x_16*x_32 + x_18*x_32 + x_22*x_32 + x_23*x_32 + x_24*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_3*x_33 + x_4*x_33 + x_6*x_33 + x_7*x_33 + x_8*x_33 + \n"
                       "        x_9*x_33 + x_12*x_33 + x_14*x_33 + x_16*x_33 + x_17*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_26*x_33 + x_29*x_33 + x_30*x_33 + x_31*x_33 + x_32*x_33 + x_3*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34\n"
                       "        + x_8*x_34 + x_9*x_34 + x_16*x_34 + x_17*x_34 + x_19*x_34 + x_23*x_34 + x_27*x_34 + x_30*x_34 + x_31*x_34 + x_1*x_35 + x_4*x_35 + x_5*x_35 + x_6*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 \n"
                       "        + x_12*x_35 + x_14*x_35 + x_16*x_35 + x_18*x_35 + x_19*x_35 + x_22*x_35 + x_24*x_35 + x_25*x_35 + x_26*x_35 + x_30*x_35 + x_33*x_35 + x_1*x_36 + x_2*x_36 + x_8*x_36 + x_9*x_36 + x_10*x_36 + \n"
                       "        x_12*x_36 + x_13*x_36 + x_17*x_36 + x_18*x_36 + x_24*x_36 + x_26*x_36 + x_31*x_36 + x_33*x_36 + x_1*x_37 + x_4*x_37 + x_5*x_37 + x_10*x_37 + x_11*x_37 + x_13*x_37 + x_16*x_37 + x_17*x_37 + \n"
                       "        x_18*x_37 + x_19*x_37 + x_20*x_37 + x_22*x_37 + x_23*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_32*x_37 + x_2*x_38 + x_4*x_38 + x_6*x_38 + x_7*x_38 + x_11*x_38 + x_12*x_38 + \n"
                       "        x_13*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_19*x_38 + x_21*x_38 + x_23*x_38 + x_24*x_38 + x_27*x_38 + x_29*x_38 + x_30*x_38 + x_32*x_38 + x_35*x_38 + x_36*x_38 + x_37*x_38 + x_5*x_39 + \n"
                       "        x_6*x_39 + x_8*x_39 + x_12*x_39 + x_14*x_39 + x_15*x_39 + x_18*x_39 + x_21*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + x_29*x_39 + x_30*x_39 + x_32*x_39 + x_34*x_39 + x_36*x_39 + \n"
                       "        x_37*x_39 + x_38*x_39 + x_4*x_40 + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_9*x_40 + x_11*x_40 + x_14*x_40 + x_15*x_40 + x_18*x_40 + x_19*x_40 + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_27*x_40\n"
                       "        + x_28*x_40 + x_30*x_40 + x_31*x_40 + x_37*x_40 + x_38*x_40 + x_39*x_40 + x_3*x_41 + x_4*x_41 + x_6*x_41 + x_11*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_18*x_41 + x_20*x_41 + x_21*x_41 + \n"
                       "        x_22*x_41 + x_24*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + x_29*x_41 + x_30*x_41 + x_32*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_37*x_41 + x_39*x_41 + x_4*x_42 + x_6*x_42 + x_7*x_42 + \n"
                       "        x_8*x_42 + x_9*x_42 + x_11*x_42 + x_12*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_22*x_42 + x_26*x_42 + x_27*x_42 + x_29*x_42 + x_31*x_42 + x_34*x_42 + x_38*x_42 + x_40*x_42 + x_41*x_42 + \n"
                       "        x_2*x_43 + x_3*x_43 + x_4*x_43 + x_6*x_43 + x_7*x_43 + x_12*x_43 + x_14*x_43 + x_17*x_43 + x_18*x_43 + x_20*x_43 + x_21*x_43 + x_26*x_43 + x_28*x_43 + x_32*x_43 + x_33*x_43 + x_35*x_43 + x_40*x_43\n"
                       "        + x_41*x_43 + x_42*x_43 + x_1*x_44 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_5*x_44 + x_7*x_44 + x_9*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_16*x_44 + x_19*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44\n"
                       "        + x_26*x_44 + x_29*x_44 + x_30*x_44 + x_32*x_44 + x_33*x_44 + x_35*x_44 + x_40*x_44 + x_41*x_44 + x_42*x_44 + x_3*x_45 + x_5*x_45 + x_6*x_45 + x_11*x_45 + x_13*x_45 + x_14*x_45 + x_17*x_45 + \n"
                       "        x_19*x_45 + x_20*x_45 + x_21*x_45 + x_25*x_45 + x_29*x_45 + x_31*x_45 + x_32*x_45 + x_34*x_45 + x_38*x_45 + x_40*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_3*x_46 + x_6*x_46 + x_8*x_46 + \n"
                       "        x_15*x_46 + x_17*x_46 + x_18*x_46 + x_20*x_46 + x_22*x_46 + x_23*x_46 + x_25*x_46 + x_26*x_46 + x_28*x_46 + x_29*x_46 + x_33*x_46 + x_35*x_46 + x_37*x_46 + x_38*x_46 + x_1*x_47 + x_2*x_47 + \n"
                       "        x_3*x_47 + x_5*x_47 + x_6*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_15*x_47 + x_16*x_47 + x_17*x_47 + x_18*x_47 + x_19*x_47 + x_21*x_47 + x_22*x_47 + \n"
                       "        x_24*x_47 + x_26*x_47 + x_32*x_47 + x_36*x_47 + x_41*x_47 + x_42*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_46*x_47 + x_1*x_48 + x_2*x_48 + x_10*x_48 + x_12*x_48 + x_15*x_48 + x_16*x_48 + \n"
                       "        x_18*x_48 + x_19*x_48 + x_20*x_48 + x_23*x_48 + x_25*x_48 + x_28*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_35*x_48 + x_38*x_48 + x_42*x_48 + x_44*x_48 + x_1*x_49 + x_4*x_49 + x_5*x_49 + \n"
                       "        x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 + x_13*x_49 + x_16*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49 + x_26*x_49 + x_28*x_49 + x_29*x_49 + x_35*x_49 + x_41*x_49 + \n"
                       "        x_44*x_49 + x_1 + x_2 + x_4 + x_8 + x_9 + x_10 + x_12 + x_13 + x_14 + x_15 + x_16 + x_18 + x_22 + x_23 + x_25 + x_26 + x_27 + x_28 + x_29 + x_31 + x_34 + x_35 + x_36 + x_39 + x_42 + x_43 + x_47,\n"
                       "    x_1*x_4 + x_3*x_4 + x_1*x_5 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_2*x_6 + x_4*x_6 + x_1*x_7 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_1*x_9 + x_3*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_1*x_10 + x_2*x_10 + x_4*x_10\n"
                       "        + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_7*x_11 + x_8*x_11 + x_3*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_5*x_13 + x_6*x_13 + \n"
                       "        x_8*x_13 + x_9*x_13 + x_10*x_13 + x_11*x_13 + x_12*x_13 + x_1*x_14 + x_2*x_14 + x_3*x_14 + x_4*x_14 + x_5*x_14 + x_6*x_14 + x_8*x_14 + x_9*x_14 + x_11*x_14 + x_2*x_15 + x_4*x_15 + x_5*x_15 + \n"
                       "        x_7*x_15 + x_8*x_15 + x_10*x_15 + x_1*x_16 + x_2*x_16 + x_4*x_16 + x_5*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_2*x_17 + x_4*x_17 + x_7*x_17 + x_8*x_17 + x_9*x_17 + x_10*x_17 + x_12*x_17 + \n"
                       "        x_1*x_18 + x_4*x_18 + x_5*x_18 + x_8*x_18 + x_9*x_18 + x_11*x_18 + x_14*x_18 + x_17*x_18 + x_2*x_19 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_6*x_19 + x_7*x_19 + x_14*x_19 + x_17*x_19 + x_18*x_19 + \n"
                       "        x_1*x_20 + x_3*x_20 + x_7*x_20 + x_8*x_20 + x_9*x_20 + x_10*x_20 + x_11*x_20 + x_13*x_20 + x_16*x_20 + x_17*x_20 + x_2*x_21 + x_5*x_21 + x_7*x_21 + x_10*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + \n"
                       "        x_18*x_21 + x_20*x_21 + x_4*x_22 + x_9*x_22 + x_12*x_22 + x_16*x_22 + x_18*x_22 + x_20*x_22 + x_1*x_23 + x_2*x_23 + x_3*x_23 + x_6*x_23 + x_8*x_23 + x_10*x_23 + x_12*x_23 + x_13*x_23 + x_14*x_23 +\n"
                       "        x_15*x_23 + x_18*x_23 + x_19*x_23 + x_22*x_23 + x_1*x_24 + x_3*x_24 + x_4*x_24 + x_5*x_24 + x_6*x_24 + x_7*x_24 + x_10*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_14*x_24 + x_16*x_24 + x_17*x_24 \n"
                       "        + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_22*x_24 + x_1*x_25 + x_6*x_25 + x_8*x_25 + x_10*x_25 + x_12*x_25 + x_14*x_25 + x_19*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_2*x_26 + x_4*x_26 + \n"
                       "        x_6*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_14*x_26 + x_15*x_26 + x_17*x_26 + x_18*x_26 + x_21*x_26 + x_22*x_26 + x_1*x_27 + x_3*x_27 + x_4*x_27 + x_6*x_27 + x_7*x_27 + x_9*x_27 + x_11*x_27 + \n"
                       "        x_13*x_27 + x_15*x_27 + x_17*x_27 + x_19*x_27 + x_22*x_27 + x_24*x_27 + x_26*x_27 + x_4*x_28 + x_7*x_28 + x_8*x_28 + x_10*x_28 + x_11*x_28 + x_13*x_28 + x_17*x_28 + x_19*x_28 + x_20*x_28 + \n"
                       "        x_21*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_27*x_28 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_8*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_14*x_29 + x_15*x_29 + x_16*x_29 + x_17*x_29 \n"
                       "        + x_20*x_29 + x_22*x_29 + x_23*x_29 + x_24*x_29 + x_1*x_30 + x_2*x_30 + x_5*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_12*x_30 + x_13*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + x_20*x_30\n"
                       "        + x_21*x_30 + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + x_28*x_30 + x_29*x_30 + x_2*x_31 + x_3*x_31 + x_4*x_31 + x_6*x_31 + x_7*x_31 + x_13*x_31 + x_14*x_31 + x_15*x_31 + \n"
                       "        x_17*x_31 + x_20*x_31 + x_21*x_31 + x_22*x_31 + x_26*x_31 + x_27*x_31 + x_29*x_31 + x_1*x_32 + x_3*x_32 + x_4*x_32 + x_5*x_32 + x_8*x_32 + x_10*x_32 + x_12*x_32 + x_13*x_32 + x_14*x_32 + x_15*x_32\n"
                       "        + x_16*x_32 + x_19*x_32 + x_22*x_32 + x_23*x_32 + x_25*x_32 + x_31*x_32 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_5*x_33 + x_6*x_33 + x_9*x_33 + x_13*x_33 + x_14*x_33 + x_16*x_33 + x_17*x_33 + \n"
                       "        x_18*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_23*x_33 + x_24*x_33 + x_26*x_33 + x_27*x_33 + x_28*x_33 + x_30*x_33 + x_31*x_33 + x_2*x_34 + x_6*x_34 + x_9*x_34 + x_10*x_34 + \n"
                       "        x_11*x_34 + x_12*x_34 + x_14*x_34 + x_15*x_34 + x_18*x_34 + x_25*x_34 + x_26*x_34 + x_27*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_3*x_35 + x_6*x_35 + x_10*x_35 + x_18*x_35 + \n"
                       "        x_19*x_35 + x_24*x_35 + x_25*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_5*x_36 + x_6*x_36 + x_7*x_36 + x_9*x_36 + x_12*x_36 + x_13*x_36 + x_16*x_36 + x_18*x_36 + x_19*x_36 + x_20*x_36 + x_22*x_36\n"
                       "        + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_30*x_36 + x_32*x_36 + x_33*x_36 + x_35*x_36 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_7*x_37 + x_8*x_37 + x_11*x_37 + x_12*x_37 + x_14*x_37 + x_17*x_37 + \n"
                       "        x_18*x_37 + x_19*x_37 + x_21*x_37 + x_23*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + x_33*x_37 + x_34*x_37 + x_36*x_37 + x_5*x_38 + x_6*x_38 + x_11*x_38 + x_16*x_38 + x_19*x_38 + \n"
                       "        x_20*x_38 + x_22*x_38 + x_26*x_38 + x_27*x_38 + x_31*x_38 + x_34*x_38 + x_36*x_38 + x_5*x_39 + x_7*x_39 + x_8*x_39 + x_10*x_39 + x_12*x_39 + x_16*x_39 + x_21*x_39 + x_22*x_39 + x_24*x_39 + \n"
                       "        x_25*x_39 + x_29*x_39 + x_31*x_39 + x_35*x_39 + x_36*x_39 + x_37*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_8*x_40 + x_10*x_40 + x_12*x_40 + x_14*x_40 + x_15*x_40 + x_16*x_40 + \n"
                       "        x_19*x_40 + x_20*x_40 + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + x_31*x_40 + x_33*x_40 + x_35*x_40 + x_36*x_40 + x_37*x_40 + x_38*x_40 + \n"
                       "        x_39*x_40 + x_1*x_41 + x_3*x_41 + x_4*x_41 + x_6*x_41 + x_7*x_41 + x_10*x_41 + x_15*x_41 + x_18*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41 + x_24*x_41 + x_25*x_41 + x_27*x_41 + x_28*x_41\n"
                       "        + x_30*x_41 + x_32*x_41 + x_37*x_41 + x_39*x_41 + x_1*x_42 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_6*x_42 + x_11*x_42 + x_12*x_42 + x_14*x_42 + x_17*x_42 + x_18*x_42 + x_20*x_42 + x_21*x_42 + \n"
                       "        x_23*x_42 + x_24*x_42 + x_26*x_42 + x_29*x_42 + x_30*x_42 + x_31*x_42 + x_33*x_42 + x_34*x_42 + x_35*x_42 + x_36*x_42 + x_37*x_42 + x_39*x_42 + x_41*x_42 + x_1*x_43 + x_4*x_43 + x_9*x_43 + \n"
                       "        x_10*x_43 + x_12*x_43 + x_14*x_43 + x_22*x_43 + x_23*x_43 + x_29*x_43 + x_33*x_43 + x_36*x_43 + x_37*x_43 + x_40*x_43 + x_1*x_44 + x_2*x_44 + x_5*x_44 + x_7*x_44 + x_8*x_44 + x_11*x_44 + x_13*x_44\n"
                       "        + x_14*x_44 + x_17*x_44 + x_19*x_44 + x_20*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_25*x_44 + x_26*x_44 + x_28*x_44 + x_30*x_44 + x_32*x_44 + x_35*x_44 + x_37*x_44 + x_40*x_44 + \n"
                       "        x_41*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + x_7*x_45 + x_8*x_45 + x_10*x_45 + x_11*x_45 + x_13*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_20*x_45 + x_21*x_45 + x_25*x_45 + x_28*x_45 + \n"
                       "        x_29*x_45 + x_32*x_45 + x_34*x_45 + x_40*x_45 + x_43*x_45 + x_2*x_46 + x_3*x_46 + x_7*x_46 + x_9*x_46 + x_12*x_46 + x_14*x_46 + x_16*x_46 + x_17*x_46 + x_19*x_46 + x_21*x_46 + x_22*x_46 + \n"
                       "        x_23*x_46 + x_27*x_46 + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_31*x_46 + x_32*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_38*x_46 + x_40*x_46 + x_44*x_46 + x_45*x_46 + x_1*x_47 + x_2*x_47 + \n"
                       "        x_3*x_47 + x_5*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + x_16*x_47 + x_17*x_47 + x_20*x_47 + x_21*x_47 + x_22*x_47 + x_24*x_47 + x_29*x_47 + x_30*x_47 + x_31*x_47 + \n"
                       "        x_34*x_47 + x_35*x_47 + x_36*x_47 + x_39*x_47 + x_40*x_47 + x_43*x_47 + x_45*x_47 + x_46*x_47 + x_2*x_48 + x_4*x_48 + x_5*x_48 + x_6*x_48 + x_7*x_48 + x_9*x_48 + x_15*x_48 + x_17*x_48 + x_18*x_48 \n"
                       "        + x_19*x_48 + x_20*x_48 + x_21*x_48 + x_23*x_48 + x_25*x_48 + x_29*x_48 + x_30*x_48 + x_33*x_48 + x_34*x_48 + x_37*x_48 + x_38*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_42*x_48 + x_44*x_48 + \n"
                       "        x_45*x_48 + x_46*x_48 + x_1*x_49 + x_2*x_49 + x_7*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 + x_16*x_49 + x_18*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49 + x_28*x_49 + x_29*x_49 + \n"
                       "        x_32*x_49 + x_35*x_49 + x_36*x_49 + x_37*x_49 + x_41*x_49 + x_42*x_49 + x_46*x_49 + x_48*x_49 + x_1 + x_3 + x_5 + x_7 + x_8 + x_11 + x_14 + x_16 + x_17 + x_22 + x_24 + x_25 + x_28 + x_31 + x_33 + \n"
                       "        x_36 + x_37 + x_38 + x_39 + x_40 + x_41 + x_44 + x_46 + x_47 + x_49,\n"
                       "    x_1*x_3 + x_2*x_3 + x_1*x_4 + x_1*x_5 + x_4*x_6 + x_2*x_7 + x_3*x_7 + x_4*x_7 + x_4*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_3*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_8*x_9 + x_1*x_10 + x_3*x_10 \n"
                       "        + x_5*x_10 + x_6*x_10 + x_7*x_10 + x_1*x_11 + x_4*x_11 + x_10*x_11 + x_1*x_12 + x_2*x_12 + x_4*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_3*x_13 + x_8*x_13 + x_9*x_13 + x_11*x_13 + \n"
                       "        x_12*x_13 + x_1*x_14 + x_2*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14 + x_8*x_14 + x_12*x_14 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_14*x_15 + x_1*x_16 + x_4*x_16 + \n"
                       "        x_5*x_16 + x_7*x_16 + x_8*x_16 + x_10*x_16 + x_11*x_16 + x_13*x_16 + x_14*x_16 + x_1*x_17 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_10*x_17 + x_12*x_17 + x_13*x_17 + x_2*x_18 + x_5*x_18 + x_7*x_18 + \n"
                       "        x_9*x_18 + x_14*x_18 + x_15*x_18 + x_16*x_18 + x_17*x_18 + x_3*x_19 + x_5*x_19 + x_6*x_19 + x_7*x_19 + x_9*x_19 + x_10*x_19 + x_13*x_19 + x_15*x_19 + x_18*x_19 + x_1*x_20 + x_4*x_20 + x_5*x_20 + \n"
                       "        x_6*x_20 + x_8*x_20 + x_9*x_20 + x_11*x_20 + x_19*x_20 + x_1*x_21 + x_3*x_21 + x_8*x_21 + x_9*x_21 + x_11*x_21 + x_13*x_21 + x_14*x_21 + x_15*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_4*x_22 + \n"
                       "        x_6*x_22 + x_8*x_22 + x_10*x_22 + x_12*x_22 + x_14*x_22 + x_17*x_22 + x_21*x_22 + x_1*x_23 + x_2*x_23 + x_5*x_23 + x_7*x_23 + x_8*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_15*x_23 +\n"
                       "        x_17*x_23 + x_20*x_23 + x_21*x_23 + x_22*x_23 + x_1*x_24 + x_3*x_24 + x_8*x_24 + x_10*x_24 + x_13*x_24 + x_14*x_24 + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_2*x_25 + x_4*x_25 + x_5*x_25 + x_7*x_25 +\n"
                       "        x_8*x_25 + x_9*x_25 + x_12*x_25 + x_13*x_25 + x_14*x_25 + x_15*x_25 + x_19*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_1*x_26 + x_5*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_11*x_26 + x_12*x_26 +\n"
                       "        x_14*x_26 + x_17*x_26 + x_18*x_26 + x_20*x_26 + x_21*x_26 + x_23*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_4*x_27 + x_6*x_27 + x_8*x_27 + x_9*x_27 + x_10*x_27 + x_14*x_27 + x_16*x_27 \n"
                       "        + x_17*x_27 + x_22*x_27 + x_25*x_27 + x_26*x_27 + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_7*x_28 + x_8*x_28 + x_10*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_18*x_28 + x_23*x_28 + x_24*x_28 + \n"
                       "        x_27*x_28 + x_1*x_29 + x_5*x_29 + x_6*x_29 + x_7*x_29 + x_9*x_29 + x_19*x_29 + x_20*x_29 + x_21*x_29 + x_24*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 + x_3*x_30 + x_4*x_30 + x_6*x_30 + x_9*x_30 + \n"
                       "        x_12*x_30 + x_14*x_30 + x_15*x_30 + x_16*x_30 + x_19*x_30 + x_21*x_30 + x_23*x_30 + x_25*x_30 + x_27*x_30 + x_28*x_30 + x_1*x_31 + x_3*x_31 + x_4*x_31 + x_5*x_31 + x_8*x_31 + x_13*x_31 + x_14*x_31\n"
                       "        + x_17*x_31 + x_18*x_31 + x_20*x_31 + x_21*x_31 + x_22*x_31 + x_23*x_31 + x_24*x_31 + x_25*x_31 + x_28*x_31 + x_29*x_31 + x_2*x_32 + x_4*x_32 + x_6*x_32 + x_8*x_32 + x_9*x_32 + x_10*x_32 + \n"
                       "        x_11*x_32 + x_13*x_32 + x_14*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_21*x_32 + x_22*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_2*x_33 + x_6*x_33 + x_7*x_33 + x_8*x_33 + x_9*x_33 + x_10*x_33 \n"
                       "        + x_13*x_33 + x_14*x_33 + x_16*x_33 + x_17*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_26*x_33 + x_32*x_33 + x_3*x_34 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_8*x_34 + x_10*x_34 + \n"
                       "        x_11*x_34 + x_16*x_34 + x_18*x_34 + x_21*x_34 + x_24*x_34 + x_25*x_34 + x_27*x_34 + x_30*x_34 + x_31*x_34 + x_2*x_35 + x_4*x_35 + x_5*x_35 + x_6*x_35 + x_9*x_35 + x_10*x_35 + x_16*x_35 + x_20*x_35\n"
                       "        + x_22*x_35 + x_24*x_35 + x_26*x_35 + x_28*x_35 + x_30*x_35 + x_32*x_35 + x_34*x_35 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_12*x_36 + x_13*x_36 + x_15*x_36 + x_16*x_36 + x_19*x_36 + \n"
                       "        x_21*x_36 + x_23*x_36 + x_24*x_36 + x_27*x_36 + x_31*x_36 + x_33*x_36 + x_1*x_37 + x_2*x_37 + x_5*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 + x_15*x_37 + x_17*x_37 + x_20*x_37 + x_22*x_37 + x_24*x_37\n"
                       "        + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_34*x_37 + x_35*x_37 + x_1*x_38 + x_4*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + x_13*x_38 + x_14*x_38 + x_16*x_38 + x_17*x_38 + x_18*x_38 + \n"
                       "        x_19*x_38 + x_22*x_38 + x_25*x_38 + x_27*x_38 + x_29*x_38 + x_32*x_38 + x_34*x_38 + x_37*x_38 + x_2*x_39 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_10*x_39 + x_11*x_39 + x_12*x_39 + x_13*x_39 + \n"
                       "        x_14*x_39 + x_15*x_39 + x_16*x_39 + x_18*x_39 + x_20*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_26*x_39 + x_27*x_39 + x_30*x_39 + x_35*x_39 + x_36*x_39 + x_37*x_39 + x_2*x_40 + x_3*x_40 + \n"
                       "        x_5*x_40 + x_7*x_40 + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_12*x_40 + x_13*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_20*x_40 + x_21*x_40 + x_24*x_40 + x_25*x_40 + x_27*x_40 + \n"
                       "        x_30*x_40 + x_31*x_40 + x_32*x_40 + x_33*x_40 + x_34*x_40 + x_35*x_40 + x_39*x_40 + x_1*x_41 + x_2*x_41 + x_3*x_41 + x_4*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + \n"
                       "        x_12*x_41 + x_14*x_41 + x_15*x_41 + x_20*x_41 + x_22*x_41 + x_27*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_38*x_41 + x_39*x_41 + x_1*x_42 + x_2*x_42 + \n"
                       "        x_3*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_9*x_42 + x_11*x_42 + x_13*x_42 + x_15*x_42 + x_18*x_42 + x_19*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_28*x_42 + x_29*x_42 + x_36*x_42 + x_37*x_42\n"
                       "        + x_38*x_42 + x_39*x_42 + x_40*x_42 + x_3*x_43 + x_5*x_43 + x_6*x_43 + x_8*x_43 + x_9*x_43 + x_10*x_43 + x_11*x_43 + x_15*x_43 + x_17*x_43 + x_21*x_43 + x_23*x_43 + x_25*x_43 + x_26*x_43 + \n"
                       "        x_27*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 + x_33*x_43 + x_34*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_38*x_43 + x_40*x_43 + x_1*x_44 + x_3*x_44 + x_4*x_44 + x_8*x_44 + x_9*x_44 + x_10*x_44\n"
                       "        + x_11*x_44 + x_12*x_44 + x_14*x_44 + x_17*x_44 + x_18*x_44 + x_19*x_44 + x_22*x_44 + x_26*x_44 + x_27*x_44 + x_30*x_44 + x_31*x_44 + x_32*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + \n"
                       "        x_37*x_44 + x_1*x_45 + x_3*x_45 + x_4*x_45 + x_6*x_45 + x_7*x_45 + x_9*x_45 + x_11*x_45 + x_16*x_45 + x_18*x_45 + x_19*x_45 + x_20*x_45 + x_22*x_45 + x_23*x_45 + x_26*x_45 + x_28*x_45 + x_29*x_45 \n"
                       "        + x_31*x_45 + x_36*x_45 + x_37*x_45 + x_40*x_45 + x_41*x_45 + x_43*x_45 + x_44*x_45 + x_2*x_46 + x_3*x_46 + x_9*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + x_15*x_46 + x_19*x_46 + x_20*x_46 + \n"
                       "        x_21*x_46 + x_29*x_46 + x_32*x_46 + x_36*x_46 + x_39*x_46 + x_40*x_46 + x_44*x_46 + x_45*x_46 + x_3*x_47 + x_4*x_47 + x_6*x_47 + x_7*x_47 + x_8*x_47 + x_9*x_47 + x_15*x_47 + x_16*x_47 + x_18*x_47 \n"
                       "        + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_26*x_47 + x_27*x_47 + x_30*x_47 + x_32*x_47 + x_34*x_47 + x_36*x_47 + x_37*x_47 + x_40*x_47 + x_41*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_3*x_48 + \n"
                       "        x_5*x_48 + x_9*x_48 + x_11*x_48 + x_13*x_48 + x_15*x_48 + x_17*x_48 + x_19*x_48 + x_22*x_48 + x_23*x_48 + x_26*x_48 + x_28*x_48 + x_29*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_34*x_48 + \n"
                       "        x_35*x_48 + x_38*x_48 + x_39*x_48 + x_46*x_48 + x_1*x_49 + x_4*x_49 + x_5*x_49 + x_6*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_14*x_49 + x_17*x_49 + x_18*x_49 + x_22*x_49 + x_24*x_49 + x_25*x_49\n"
                       "        + x_27*x_49 + x_29*x_49 + x_31*x_49 + x_34*x_49 + x_35*x_49 + x_38*x_49 + x_41*x_49 + x_42*x_49 + x_46*x_49 + x_48*x_49 + x_2 + x_3 + x_5 + x_6 + x_8 + x_9 + x_11 + x_13 + x_14 + x_15 + x_16 + \n"
                       "        x_18 + x_20 + x_22 + x_23 + x_25 + x_26 + x_30 + x_31 + x_32 + x_37 + x_38 + x_40 + x_42 + x_43 + x_44 + x_46 + x_48,\n"
                       "    x_2*x_3 + x_2*x_4 + x_2*x_5 + x_2*x_6 + x_3*x_6 + x_3*x_7 + x_5*x_7 + x_2*x_8 + x_1*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + x_7*x_9 + x_1*x_10 + x_3*x_10 + x_8*x_10 + x_9*x_10 + x_2*x_11 + x_3*x_11 + \n"
                       "        x_9*x_11 + x_10*x_11 + x_1*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_7*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_8*x_13 + x_10*x_13 + x_2*x_14 + x_4*x_14 + x_5*x_14 + \n"
                       "        x_12*x_14 + x_13*x_14 + x_1*x_15 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_6*x_15 + x_8*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_12*x_15 + x_13*x_15 + x_14*x_15 + x_1*x_16 + x_2*x_16 + x_4*x_16 + \n"
                       "        x_5*x_16 + x_6*x_16 + x_7*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_2*x_17 + x_4*x_17 + x_5*x_17 + x_7*x_17 + x_8*x_17 + x_10*x_17 + x_11*x_17 + x_12*x_17 + x_14*x_17 + \n"
                       "        x_15*x_17 + x_16*x_17 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_7*x_18 + x_9*x_18 + x_11*x_18 + x_12*x_18 + x_13*x_18 + x_14*x_18 + x_15*x_18 + x_17*x_18 + x_3*x_19 + x_4*x_19 + x_5*x_19 + \n"
                       "        x_6*x_19 + x_7*x_19 + x_10*x_19 + x_11*x_19 + x_13*x_19 + x_1*x_20 + x_3*x_20 + x_5*x_20 + x_8*x_20 + x_11*x_20 + x_14*x_20 + x_15*x_20 + x_19*x_20 + x_7*x_21 + x_8*x_21 + x_11*x_21 + x_16*x_21 + \n"
                       "        x_17*x_21 + x_19*x_21 + x_1*x_22 + x_6*x_22 + x_9*x_22 + x_11*x_22 + x_12*x_22 + x_13*x_22 + x_14*x_22 + x_16*x_22 + x_18*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_4*x_23 + x_7*x_23 + x_8*x_23 +\n"
                       "        x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_15*x_23 + x_16*x_23 + x_18*x_23 + x_19*x_23 + x_20*x_23 + x_22*x_23 + x_2*x_24 + x_3*x_24 + x_10*x_24 + x_11*x_24 + x_13*x_24 + x_14*x_24 + \n"
                       "        x_15*x_24 + x_16*x_24 + x_17*x_24 + x_18*x_24 + x_21*x_24 + x_22*x_24 + x_1*x_25 + x_2*x_25 + x_3*x_25 + x_4*x_25 + x_5*x_25 + x_10*x_25 + x_11*x_25 + x_12*x_25 + x_14*x_25 + x_15*x_25 + x_18*x_25\n"
                       "        + x_19*x_25 + x_22*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_4*x_26 + x_6*x_26 + x_8*x_26 + x_9*x_26 + x_10*x_26 + x_11*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_16*x_26 + x_18*x_26 + x_19*x_26\n"
                       "        + x_20*x_26 + x_22*x_26 + x_24*x_26 + x_25*x_26 + x_2*x_27 + x_3*x_27 + x_4*x_27 + x_5*x_27 + x_9*x_27 + x_10*x_27 + x_11*x_27 + x_14*x_27 + x_18*x_27 + x_22*x_27 + x_23*x_27 + x_25*x_27 + \n"
                       "        x_26*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_6*x_28 + x_7*x_28 + x_10*x_28 + x_12*x_28 + x_14*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_20*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_26*x_28\n"
                       "        + x_27*x_28 + x_3*x_29 + x_5*x_29 + x_6*x_29 + x_9*x_29 + x_11*x_29 + x_13*x_29 + x_14*x_29 + x_19*x_29 + x_21*x_29 + x_26*x_29 + x_6*x_30 + x_7*x_30 + x_9*x_30 + x_10*x_30 + x_15*x_30 + x_17*x_30\n"
                       "        + x_19*x_30 + x_22*x_30 + x_24*x_30 + x_26*x_30 + x_29*x_30 + x_1*x_31 + x_3*x_31 + x_4*x_31 + x_7*x_31 + x_10*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_19*x_31 + x_21*x_31 + x_26*x_31 + \n"
                       "        x_27*x_31 + x_28*x_31 + x_29*x_31 + x_1*x_32 + x_7*x_32 + x_9*x_32 + x_10*x_32 + x_11*x_32 + x_17*x_32 + x_20*x_32 + x_21*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + \n"
                       "        x_30*x_32 + x_31*x_32 + x_1*x_33 + x_2*x_33 + x_3*x_33 + x_5*x_33 + x_6*x_33 + x_8*x_33 + x_9*x_33 + x_10*x_33 + x_13*x_33 + x_14*x_33 + x_15*x_33 + x_17*x_33 + x_18*x_33 + x_22*x_33 + x_24*x_33 +\n"
                       "        x_27*x_33 + x_29*x_33 + x_31*x_33 + x_32*x_33 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_6*x_34 + x_8*x_34 + x_11*x_34 + x_13*x_34 + x_17*x_34 + x_21*x_34 + x_24*x_34 + x_25*x_34 + x_28*x_34 + x_29*x_34\n"
                       "        + x_32*x_34 + x_2*x_35 + x_3*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_15*x_35 + x_16*x_35 + x_17*x_35 + x_18*x_35 + x_22*x_35 + x_23*x_35 + x_24*x_35 + x_29*x_35 + x_30*x_35 + x_34*x_35 + \n"
                       "        x_2*x_36 + x_3*x_36 + x_4*x_36 + x_5*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 + x_10*x_36 + x_11*x_36 + x_14*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + x_18*x_36 + x_20*x_36 + x_22*x_36 + x_25*x_36 +\n"
                       "        x_27*x_36 + x_28*x_36 + x_29*x_36 + x_30*x_36 + x_31*x_36 + x_1*x_37 + x_5*x_37 + x_6*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 + x_14*x_37 + x_17*x_37 + x_18*x_37 \n"
                       "        + x_19*x_37 + x_21*x_37 + x_22*x_37 + x_23*x_37 + x_25*x_37 + x_26*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + x_32*x_37 + x_36*x_37 + x_3*x_38 + x_11*x_38 + x_12*x_38 + x_14*x_38 + \n"
                       "        x_15*x_38 + x_16*x_38 + x_17*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + x_25*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_34*x_38 + x_3*x_39 + x_8*x_39 + x_10*x_39 + x_11*x_39 + x_14*x_39 + \n"
                       "        x_16*x_39 + x_17*x_39 + x_18*x_39 + x_19*x_39 + x_21*x_39 + x_22*x_39 + x_25*x_39 + x_26*x_39 + x_28*x_39 + x_29*x_39 + x_33*x_39 + x_35*x_39 + x_37*x_39 + x_1*x_40 + x_4*x_40 + x_5*x_40 + \n"
                       "        x_7*x_40 + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_12*x_40 + x_14*x_40 + x_15*x_40 + x_18*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + x_26*x_40 + x_27*x_40 + x_29*x_40 + x_33*x_40 + x_35*x_40 + \n"
                       "        x_37*x_40 + x_38*x_40 + x_1*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + x_12*x_41 + x_14*x_41 + x_17*x_41 + x_18*x_41 + x_23*x_41 + x_25*x_41 + x_29*x_41 + x_30*x_41 \n"
                       "        + x_31*x_41 + x_33*x_41 + x_34*x_41 + x_35*x_41 + x_36*x_41 + x_37*x_41 + x_2*x_42 + x_5*x_42 + x_7*x_42 + x_9*x_42 + x_11*x_42 + x_13*x_42 + x_14*x_42 + x_16*x_42 + x_19*x_42 + x_21*x_42 + \n"
                       "        x_22*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_33*x_42 + x_34*x_42 + x_36*x_42 + x_37*x_42 + x_38*x_42 + x_39*x_42 + x_41*x_42 + x_1*x_43 + \n"
                       "        x_2*x_43 + x_3*x_43 + x_4*x_43 + x_6*x_43 + x_7*x_43 + x_8*x_43 + x_9*x_43 + x_11*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_18*x_43 + x_19*x_43 + x_25*x_43 + x_27*x_43 +\n"
                       "        x_28*x_43 + x_32*x_43 + x_33*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_42*x_43 + x_2*x_44 + x_4*x_44 + x_6*x_44 + x_8*x_44 + x_9*x_44 + x_11*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44\n"
                       "        + x_16*x_44 + x_18*x_44 + x_20*x_44 + x_23*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_29*x_44 + x_31*x_44 + x_32*x_44 + x_33*x_44 + x_37*x_44 + x_39*x_44 + x_40*x_44 + x_41*x_44 + \n"
                       "        x_42*x_44 + x_43*x_44 + x_1*x_45 + x_3*x_45 + x_4*x_45 + x_5*x_45 + x_8*x_45 + x_9*x_45 + x_11*x_45 + x_13*x_45 + x_15*x_45 + x_16*x_45 + x_19*x_45 + x_22*x_45 + x_23*x_45 + x_25*x_45 + x_26*x_45 \n"
                       "        + x_27*x_45 + x_28*x_45 + x_33*x_45 + x_39*x_45 + x_40*x_45 + x_42*x_45 + x_2*x_46 + x_3*x_46 + x_4*x_46 + x_8*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + x_14*x_46 + x_16*x_46 + x_18*x_46 + \n"
                       "        x_19*x_46 + x_20*x_46 + x_21*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_27*x_46 + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_31*x_46 + x_32*x_46 + x_35*x_46 + x_39*x_46 + x_41*x_46 + x_42*x_46 + \n"
                       "        x_43*x_46 + x_44*x_46 + x_2*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + x_14*x_47 + x_15*x_47 + x_18*x_47 + x_19*x_47 + x_23*x_47 + x_24*x_47 + x_25*x_47 + x_26*x_47 + \n"
                       "        x_28*x_47 + x_30*x_47 + x_32*x_47 + x_34*x_47 + x_35*x_47 + x_38*x_47 + x_40*x_47 + x_41*x_47 + x_42*x_47 + x_3*x_48 + x_7*x_48 + x_8*x_48 + x_10*x_48 + x_14*x_48 + x_15*x_48 + x_17*x_48 + \n"
                       "        x_18*x_48 + x_19*x_48 + x_21*x_48 + x_22*x_48 + x_23*x_48 + x_24*x_48 + x_28*x_48 + x_29*x_48 + x_32*x_48 + x_33*x_48 + x_34*x_48 + x_35*x_48 + x_38*x_48 + x_39*x_48 + x_41*x_48 + x_42*x_48 + \n"
                       "        x_44*x_48 + x_47*x_48 + x_1*x_49 + x_3*x_49 + x_4*x_49 + x_9*x_49 + x_13*x_49 + x_16*x_49 + x_18*x_49 + x_19*x_49 + x_20*x_49 + x_22*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49 + \n"
                       "        x_29*x_49 + x_33*x_49 + x_36*x_49 + x_42*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_3 + x_4 + x_8 + x_12 + x_18 + x_21 + x_22 + x_23 + x_24 + x_25 + x_26 + x_27 + x_29 + x_30 + x_32 + x_33\n"
                       "        + x_35 + x_37 + x_38 + x_39 + x_41 + x_42 + x_45 + x_46 + x_48 + x_49,\n"
                       "    x_1*x_3 + x_2*x_3 + x_1*x_4 + x_3*x_4 + x_2*x_5 + x_3*x_5 + x_4*x_5 + x_1*x_6 + x_4*x_6 + x_5*x_6 + x_4*x_7 + x_6*x_7 + x_1*x_8 + x_4*x_8 + x_5*x_8 + x_7*x_8 + x_1*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + \n"
                       "        x_7*x_9 + x_2*x_10 + x_4*x_10 + x_6*x_10 + x_7*x_10 + x_2*x_11 + x_4*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_3*x_12 + x_5*x_12 + x_10*x_12 + x_1*x_13 + x_4*x_13 + x_5*x_13 + x_8*x_13 + \n"
                       "        x_12*x_13 + x_3*x_14 + x_5*x_14 + x_8*x_14 + x_9*x_14 + x_10*x_14 + x_1*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + x_9*x_15 + x_13*x_15 + x_2*x_16 + x_3*x_16 + x_5*x_16 + x_6*x_16 + x_7*x_16 + \n"
                       "        x_9*x_16 + x_12*x_16 + x_13*x_16 + x_3*x_17 + x_4*x_17 + x_6*x_17 + x_7*x_17 + x_8*x_17 + x_9*x_17 + x_10*x_17 + x_12*x_17 + x_14*x_17 + x_15*x_17 + x_1*x_18 + x_6*x_18 + x_7*x_18 + x_11*x_18 + \n"
                       "        x_13*x_18 + x_14*x_18 + x_16*x_18 + x_1*x_19 + x_2*x_19 + x_5*x_19 + x_6*x_19 + x_7*x_19 + x_9*x_19 + x_18*x_19 + x_2*x_20 + x_10*x_20 + x_12*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 +\n"
                       "        x_18*x_20 + x_19*x_20 + x_1*x_21 + x_2*x_21 + x_3*x_21 + x_4*x_21 + x_6*x_21 + x_7*x_21 + x_8*x_21 + x_9*x_21 + x_14*x_21 + x_15*x_21 + x_17*x_21 + x_18*x_21 + x_19*x_21 + x_20*x_21 + x_1*x_22 + \n"
                       "        x_3*x_22 + x_6*x_22 + x_8*x_22 + x_10*x_22 + x_11*x_22 + x_15*x_22 + x_17*x_22 + x_19*x_22 + x_21*x_22 + x_1*x_23 + x_3*x_23 + x_4*x_23 + x_5*x_23 + x_7*x_23 + x_10*x_23 + x_11*x_23 + x_13*x_23 + \n"
                       "        x_14*x_23 + x_17*x_23 + x_20*x_23 + x_21*x_23 + x_1*x_24 + x_4*x_24 + x_5*x_24 + x_6*x_24 + x_7*x_24 + x_9*x_24 + x_10*x_24 + x_13*x_24 + x_14*x_24 + x_15*x_24 + x_16*x_24 + x_18*x_24 + x_19*x_24 \n"
                       "        + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_1*x_25 + x_2*x_25 + x_3*x_25 + x_5*x_25 + x_8*x_25 + x_13*x_25 + x_15*x_25 + x_16*x_25 + x_17*x_25 + x_24*x_25 + x_1*x_26 + x_4*x_26 + x_5*x_26 + x_6*x_26 +\n"
                       "        x_7*x_26 + x_11*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_21*x_26 + x_22*x_26 + x_1*x_27 + x_2*x_27 + x_8*x_27 + x_9*x_27 + x_13*x_27 + x_15*x_27 + x_16*x_27\n"
                       "        + x_17*x_27 + x_18*x_27 + x_20*x_27 + x_21*x_27 + x_22*x_27 + x_23*x_27 + x_25*x_27 + x_26*x_27 + x_2*x_28 + x_3*x_28 + x_5*x_28 + x_6*x_28 + x_7*x_28 + x_10*x_28 + x_11*x_28 + x_15*x_28 + \n"
                       "        x_16*x_28 + x_18*x_28 + x_21*x_28 + x_22*x_28 + x_24*x_28 + x_26*x_28 + x_1*x_29 + x_3*x_29 + x_4*x_29 + x_6*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_15*x_29 + x_16*x_29 + x_18*x_29 + x_21*x_29\n"
                       "        + x_23*x_29 + x_27*x_29 + x_2*x_30 + x_4*x_30 + x_6*x_30 + x_7*x_30 + x_8*x_30 + x_10*x_30 + x_11*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + x_19*x_30 + x_23*x_30 + x_24*x_30 + x_26*x_30 + \n"
                       "        x_27*x_30 + x_29*x_30 + x_2*x_31 + x_6*x_31 + x_9*x_31 + x_10*x_31 + x_11*x_31 + x_13*x_31 + x_15*x_31 + x_16*x_31 + x_18*x_31 + x_19*x_31 + x_21*x_31 + x_23*x_31 + x_24*x_31 + x_25*x_31 + \n"
                       "        x_27*x_31 + x_28*x_31 + x_29*x_31 + x_1*x_32 + x_5*x_32 + x_6*x_32 + x_7*x_32 + x_8*x_32 + x_9*x_32 + x_10*x_32 + x_13*x_32 + x_14*x_32 + x_15*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 \n"
                       "        + x_21*x_32 + x_22*x_32 + x_23*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_30*x_32 + x_4*x_33 + x_7*x_33 + x_8*x_33 + x_9*x_33 + x_12*x_33 + x_13*x_33 + x_15*x_33 + x_16*x_33 + x_17*x_33 + \n"
                       "        x_21*x_33 + x_25*x_33 + x_27*x_33 + x_28*x_33 + x_32*x_33 + x_3*x_34 + x_4*x_34 + x_7*x_34 + x_9*x_34 + x_11*x_34 + x_12*x_34 + x_15*x_34 + x_17*x_34 + x_21*x_34 + x_27*x_34 + x_31*x_34 + \n"
                       "        x_33*x_34 + x_1*x_35 + x_2*x_35 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_8*x_35 + x_9*x_35 + x_11*x_35 + x_14*x_35 + x_16*x_35 + x_18*x_35 + x_21*x_35 + x_26*x_35 + x_27*x_35 + x_28*x_35 + x_30*x_35 +\n"
                       "        x_31*x_35 + x_32*x_35 + x_34*x_35 + x_2*x_36 + x_3*x_36 + x_9*x_36 + x_15*x_36 + x_17*x_36 + x_19*x_36 + x_26*x_36 + x_33*x_36 + x_1*x_37 + x_3*x_37 + x_5*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 \n"
                       "        + x_14*x_37 + x_15*x_37 + x_16*x_37 + x_18*x_37 + x_19*x_37 + x_23*x_37 + x_29*x_37 + x_30*x_37 + x_32*x_37 + x_34*x_37 + x_35*x_37 + x_2*x_38 + x_7*x_38 + x_9*x_38 + x_10*x_38 + x_11*x_38 + \n"
                       "        x_12*x_38 + x_13*x_38 + x_15*x_38 + x_16*x_38 + x_20*x_38 + x_26*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_33*x_38 + x_34*x_38 + x_2*x_39 + x_3*x_39 + x_4*x_39 + \n"
                       "        x_5*x_39 + x_6*x_39 + x_9*x_39 + x_12*x_39 + x_16*x_39 + x_17*x_39 + x_19*x_39 + x_20*x_39 + x_22*x_39 + x_24*x_39 + x_27*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_32*x_39 + x_33*x_39 + \n"
                       "        x_35*x_39 + x_36*x_39 + x_37*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_8*x_40 + x_13*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_19*x_40 + x_20*x_40 + x_21*x_40 + \n"
                       "        x_23*x_40 + x_24*x_40 + x_26*x_40 + x_28*x_40 + x_29*x_40 + x_30*x_40 + x_36*x_40 + x_38*x_40 + x_3*x_41 + x_5*x_41 + x_9*x_41 + x_10*x_41 + x_17*x_41 + x_24*x_41 + x_25*x_41 + x_26*x_41 + \n"
                       "        x_27*x_41 + x_28*x_41 + x_29*x_41 + x_30*x_41 + x_36*x_41 + x_38*x_41 + x_39*x_41 + x_1*x_42 + x_4*x_42 + x_7*x_42 + x_8*x_42 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_13*x_42 + x_16*x_42 + \n"
                       "        x_20*x_42 + x_21*x_42 + x_22*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_31*x_42 + x_34*x_42 + x_35*x_42 + x_39*x_42 + x_1*x_43 + x_4*x_43 + x_6*x_43 + \n"
                       "        x_7*x_43 + x_8*x_43 + x_9*x_43 + x_11*x_43 + x_16*x_43 + x_20*x_43 + x_22*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 + x_28*x_43 + x_31*x_43 + x_32*x_43 + x_35*x_43 + x_36*x_43 + x_40*x_43 + \n"
                       "        x_42*x_43 + x_2*x_44 + x_4*x_44 + x_5*x_44 + x_7*x_44 + x_11*x_44 + x_14*x_44 + x_17*x_44 + x_18*x_44 + x_19*x_44 + x_20*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 + x_25*x_44 + x_28*x_44 + \n"
                       "        x_30*x_44 + x_32*x_44 + x_34*x_44 + x_37*x_44 + x_41*x_44 + x_1*x_45 + x_2*x_45 + x_3*x_45 + x_4*x_45 + x_10*x_45 + x_11*x_45 + x_13*x_45 + x_15*x_45 + x_17*x_45 + x_18*x_45 + x_19*x_45 + \n"
                       "        x_22*x_45 + x_23*x_45 + x_25*x_45 + x_27*x_45 + x_29*x_45 + x_31*x_45 + x_32*x_45 + x_37*x_45 + x_39*x_45 + x_41*x_45 + x_1*x_46 + x_2*x_46 + x_4*x_46 + x_6*x_46 + x_9*x_46 + x_11*x_46 + x_13*x_46\n"
                       "        + x_14*x_46 + x_15*x_46 + x_18*x_46 + x_19*x_46 + x_26*x_46 + x_28*x_46 + x_30*x_46 + x_31*x_46 + x_32*x_46 + x_35*x_46 + x_36*x_46 + x_37*x_46 + x_38*x_46 + x_39*x_46 + x_41*x_46 + x_45*x_46 + \n"
                       "        x_6*x_47 + x_8*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_13*x_47 + x_15*x_47 + x_18*x_47 + x_19*x_47 + x_25*x_47 + x_26*x_47 + x_27*x_47 + x_30*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + \n"
                       "        x_36*x_47 + x_37*x_47 + x_40*x_47 + x_42*x_47 + x_44*x_47 + x_46*x_47 + x_2*x_48 + x_3*x_48 + x_5*x_48 + x_7*x_48 + x_9*x_48 + x_11*x_48 + x_15*x_48 + x_18*x_48 + x_19*x_48 + x_22*x_48 + x_25*x_48\n"
                       "        + x_26*x_48 + x_27*x_48 + x_30*x_48 + x_31*x_48 + x_34*x_48 + x_36*x_48 + x_37*x_48 + x_38*x_48 + x_41*x_48 + x_42*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + x_3*x_49 + x_5*x_49 + x_6*x_49 + \n"
                       "        x_8*x_49 + x_9*x_49 + x_12*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_17*x_49 + x_19*x_49 + x_23*x_49 + x_24*x_49 + x_27*x_49 + x_28*x_49 + x_31*x_49 + x_32*x_49 + x_33*x_49 + x_37*x_49 + \n"
                       "        x_38*x_49 + x_40*x_49 + x_42*x_49 + x_44*x_49 + x_47*x_49 + x_1 + x_4 + x_5 + x_6 + x_10 + x_12 + x_13 + x_14 + x_15 + x_17 + x_18 + x_20 + x_21 + x_25 + x_26 + x_27 + x_29 + x_30 + x_38 + x_43 + \n"
                       "        x_44 + x_46 + x_48 + x_49,\n"
                       "    x_1*x_2 + x_1*x_3 + x_2*x_5 + x_3*x_5 + x_2*x_6 + x_3*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_6*x_7 + x_2*x_8 + x_3*x_8 + x_4*x_8 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_5*x_9 + x_7*x_9 + x_2*x_10 + x_5*x_10 \n"
                       "        + x_7*x_10 + x_8*x_10 + x_1*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_8*x_11 + x_10*x_11 + x_1*x_12 + x_2*x_12 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_10*x_12 + x_1*x_13 + x_2*x_13 + x_12*x_13 + \n"
                       "        x_2*x_14 + x_3*x_14 + x_4*x_14 + x_6*x_14 + x_9*x_14 + x_12*x_14 + x_13*x_14 + x_2*x_15 + x_3*x_15 + x_5*x_15 + x_7*x_15 + x_9*x_15 + x_10*x_15 + x_12*x_15 + x_14*x_15 + x_1*x_16 + x_2*x_16 + \n"
                       "        x_4*x_16 + x_5*x_16 + x_6*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_2*x_17 + x_4*x_17 + x_5*x_17 + x_8*x_17 + x_9*x_17 + x_10*x_17 + x_11*x_17 + \n"
                       "        x_12*x_17 + x_13*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_7*x_18 + x_8*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_14*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + \n"
                       "        x_2*x_19 + x_5*x_19 + x_9*x_19 + x_10*x_19 + x_11*x_19 + x_13*x_19 + x_14*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_1*x_20 + x_2*x_20 + x_3*x_20 + x_4*x_20 + x_5*x_20 + x_6*x_20 + x_8*x_20 + \n"
                       "        x_14*x_20 + x_15*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20 + x_2*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_8*x_21 + x_9*x_21 + x_20*x_21 + x_1*x_22 + x_2*x_22 + x_5*x_22 + x_7*x_22 + x_11*x_22 + \n"
                       "        x_12*x_22 + x_13*x_22 + x_14*x_22 + x_16*x_22 + x_17*x_22 + x_20*x_22 + x_2*x_23 + x_5*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_14*x_23 + x_17*x_23 + x_18*x_23 + x_19*x_23 + \n"
                       "        x_20*x_23 + x_1*x_24 + x_10*x_24 + x_13*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_23*x_24 + x_2*x_25 + x_8*x_25 + x_10*x_25 + x_11*x_25 + x_14*x_25 + x_15*x_25 + x_18*x_25 + \n"
                       "        x_20*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_3*x_26 + x_8*x_26 + x_10*x_26 + x_14*x_26 + x_15*x_26 + x_17*x_26 + x_18*x_26 + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_23*x_26 + x_25*x_26 + \n"
                       "        x_1*x_27 + x_2*x_27 + x_6*x_27 + x_7*x_27 + x_9*x_27 + x_10*x_27 + x_12*x_27 + x_13*x_27 + x_18*x_27 + x_21*x_27 + x_23*x_27 + x_26*x_27 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_7*x_28 + x_10*x_28 + \n"
                       "        x_12*x_28 + x_13*x_28 + x_19*x_28 + x_23*x_28 + x_1*x_29 + x_2*x_29 + x_4*x_29 + x_5*x_29 + x_6*x_29 + x_7*x_29 + x_8*x_29 + x_10*x_29 + x_11*x_29 + x_12*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 +\n"
                       "        x_21*x_29 + x_23*x_29 + x_24*x_29 + x_25*x_29 + x_27*x_29 + x_1*x_30 + x_2*x_30 + x_4*x_30 + x_5*x_30 + x_8*x_30 + x_12*x_30 + x_13*x_30 + x_16*x_30 + x_21*x_30 + x_22*x_30 + x_23*x_30 + x_24*x_30\n"
                       "        + x_25*x_30 + x_28*x_30 + x_1*x_31 + x_3*x_31 + x_6*x_31 + x_7*x_31 + x_10*x_31 + x_11*x_31 + x_12*x_31 + x_14*x_31 + x_15*x_31 + x_17*x_31 + x_21*x_31 + x_22*x_31 + x_23*x_31 + x_25*x_31 + \n"
                       "        x_26*x_31 + x_1*x_32 + x_5*x_32 + x_10*x_32 + x_13*x_32 + x_16*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32 + x_29*x_32 + x_2*x_33 + x_3*x_33 + x_6*x_33\n"
                       "        + x_7*x_33 + x_8*x_33 + x_11*x_33 + x_15*x_33 + x_16*x_33 + x_20*x_33 + x_22*x_33 + x_28*x_33 + x_29*x_33 + x_32*x_33 + x_3*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_9*x_34 + x_12*x_34 + x_13*x_34\n"
                       "        + x_17*x_34 + x_18*x_34 + x_19*x_34 + x_23*x_34 + x_24*x_34 + x_27*x_34 + x_33*x_34 + x_1*x_35 + x_3*x_35 + x_6*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_11*x_35 + x_12*x_35 + x_15*x_35 + \n"
                       "        x_18*x_35 + x_21*x_35 + x_22*x_35 + x_24*x_35 + x_26*x_35 + x_27*x_35 + x_29*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_4*x_36 + x_5*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 + x_10*x_36 + x_16*x_36 +\n"
                       "        x_17*x_36 + x_18*x_36 + x_19*x_36 + x_20*x_36 + x_23*x_36 + x_24*x_36 + x_26*x_36 + x_28*x_36 + x_29*x_36 + x_30*x_36 + x_32*x_36 + x_33*x_36 + x_34*x_36 + x_35*x_36 + x_2*x_37 + x_3*x_37 + \n"
                       "        x_4*x_37 + x_5*x_37 + x_6*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 + x_14*x_37 + x_17*x_37 + x_18*x_37 + x_21*x_37 + x_23*x_37 + x_26*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + x_31*x_37 + \n"
                       "        x_32*x_37 + x_33*x_37 + x_34*x_37 + x_35*x_37 + x_36*x_37 + x_1*x_38 + x_3*x_38 + x_4*x_38 + x_5*x_38 + x_6*x_38 + x_14*x_38 + x_15*x_38 + x_17*x_38 + x_18*x_38 + x_19*x_38 + x_22*x_38 + x_23*x_38\n"
                       "        + x_25*x_38 + x_27*x_38 + x_32*x_38 + x_33*x_38 + x_34*x_38 + x_35*x_38 + x_2*x_39 + x_3*x_39 + x_5*x_39 + x_6*x_39 + x_7*x_39 + x_9*x_39 + x_11*x_39 + x_12*x_39 + x_13*x_39 + x_14*x_39 + \n"
                       "        x_16*x_39 + x_17*x_39 + x_20*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_27*x_39 + x_28*x_39 + x_30*x_39 + x_31*x_39 + x_33*x_39 + x_36*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_7*x_40 + x_8*x_40\n"
                       "        + x_11*x_40 + x_14*x_40 + x_17*x_40 + x_20*x_40 + x_21*x_40 + x_22*x_40 + x_25*x_40 + x_26*x_40 + x_28*x_40 + x_33*x_40 + x_36*x_40 + x_38*x_40 + x_39*x_40 + x_1*x_41 + x_3*x_41 + x_5*x_41 + \n"
                       "        x_6*x_41 + x_7*x_41 + x_9*x_41 + x_10*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_18*x_41 + x_19*x_41 + x_20*x_41 + x_22*x_41 + x_24*x_41 + x_25*x_41 + x_26*x_41 + \n"
                       "        x_27*x_41 + x_31*x_41 + x_32*x_41 + x_34*x_41 + x_37*x_41 + x_1*x_42 + x_3*x_42 + x_4*x_42 + x_8*x_42 + x_10*x_42 + x_12*x_42 + x_14*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + \n"
                       "        x_20*x_42 + x_21*x_42 + x_25*x_42 + x_26*x_42 + x_29*x_42 + x_30*x_42 + x_34*x_42 + x_37*x_42 + x_38*x_42 + x_41*x_42 + x_7*x_43 + x_8*x_43 + x_9*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + \n"
                       "        x_15*x_43 + x_16*x_43 + x_19*x_43 + x_21*x_43 + x_23*x_43 + x_27*x_43 + x_28*x_43 + x_32*x_43 + x_33*x_43 + x_34*x_43 + x_36*x_43 + x_37*x_43 + x_40*x_43 + x_42*x_43 + x_1*x_44 + x_2*x_44 + \n"
                       "        x_3*x_44 + x_6*x_44 + x_8*x_44 + x_11*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_16*x_44 + x_17*x_44 + x_18*x_44 + x_20*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44 + x_25*x_44 + x_26*x_44 + \n"
                       "        x_27*x_44 + x_28*x_44 + x_29*x_44 + x_30*x_44 + x_32*x_44 + x_34*x_44 + x_36*x_44 + x_37*x_44 + x_38*x_44 + x_40*x_44 + x_41*x_44 + x_43*x_44 + x_1*x_45 + x_2*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45\n"
                       "        + x_10*x_45 + x_11*x_45 + x_12*x_45 + x_17*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_24*x_45 + x_28*x_45 + x_35*x_45 + x_37*x_45 + x_38*x_45 + x_39*x_45 + x_1*x_46 + x_2*x_46 + \n"
                       "        x_3*x_46 + x_4*x_46 + x_5*x_46 + x_6*x_46 + x_7*x_46 + x_8*x_46 + x_9*x_46 + x_11*x_46 + x_14*x_46 + x_15*x_46 + x_16*x_46 + x_18*x_46 + x_19*x_46 + x_22*x_46 + x_23*x_46 + x_26*x_46 + x_28*x_46 +\n"
                       "        x_29*x_46 + x_31*x_46 + x_32*x_46 + x_34*x_46 + x_37*x_46 + x_38*x_46 + x_40*x_46 + x_42*x_46 + x_45*x_46 + x_2*x_47 + x_4*x_47 + x_5*x_47 + x_9*x_47 + x_10*x_47 + x_11*x_47 + x_12*x_47 + \n"
                       "        x_13*x_47 + x_14*x_47 + x_16*x_47 + x_17*x_47 + x_19*x_47 + x_22*x_47 + x_25*x_47 + x_27*x_47 + x_28*x_47 + x_29*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_36*x_47 + x_40*x_47 + x_42*x_47 + \n"
                       "        x_43*x_47 + x_44*x_47 + x_46*x_47 + x_1*x_48 + x_2*x_48 + x_3*x_48 + x_5*x_48 + x_7*x_48 + x_8*x_48 + x_9*x_48 + x_13*x_48 + x_15*x_48 + x_18*x_48 + x_22*x_48 + x_24*x_48 + x_27*x_48 + x_29*x_48 +\n"
                       "        x_34*x_48 + x_35*x_48 + x_36*x_48 + x_38*x_48 + x_39*x_48 + x_42*x_48 + x_43*x_48 + x_46*x_48 + x_1*x_49 + x_2*x_49 + x_4*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_11*x_49 + x_12*x_49 \n"
                       "        + x_15*x_49 + x_16*x_49 + x_18*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49 + x_24*x_49 + x_25*x_49 + x_26*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + x_38*x_49 + \n"
                       "        x_39*x_49 + x_41*x_49 + x_42*x_49 + x_44*x_49 + x_45*x_49 + x_47*x_49 + x_48*x_49 + x_6 + x_10 + x_11 + x_14 + x_15 + x_17 + x_18 + x_20 + x_21 + x_23 + x_26 + x_29 + x_30 + x_31 + x_34 + x_37 + \n"
                       "        x_38 + x_39 + x_41 + x_42 + x_43 + x_45 + x_47 + x_48,\n"
                       "    x_1*x_2 + x_2*x_4 + x_3*x_4 + x_2*x_5 + x_1*x_6 + x_1*x_7 + x_5*x_7 + x_1*x_8 + x_2*x_8 + x_4*x_8 + x_6*x_8 + x_7*x_9 + x_8*x_9 + x_1*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + \n"
                       "        x_5*x_11 + x_7*x_11 + x_8*x_11 + x_9*x_11 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_4*x_13 + x_5*x_13 + x_8*x_13 + x_10*x_13 + x_11*x_13 + x_12*x_13 + \n"
                       "        x_2*x_14 + x_3*x_14 + x_5*x_14 + x_9*x_14 + x_11*x_14 + x_1*x_15 + x_2*x_15 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_7*x_15 + x_8*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_13*x_15 + x_2*x_16 + \n"
                       "        x_6*x_16 + x_8*x_16 + x_9*x_16 + x_14*x_16 + x_4*x_17 + x_6*x_17 + x_8*x_17 + x_13*x_17 + x_14*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_7*x_18 + x_8*x_18 + x_10*x_18 + \n"
                       "        x_11*x_18 + x_12*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + x_2*x_19 + x_3*x_19 + x_6*x_19 + x_8*x_19 + x_11*x_19 + x_13*x_19 + x_14*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19 + x_3*x_20 + x_4*x_20 +\n"
                       "        x_6*x_20 + x_7*x_20 + x_8*x_20 + x_13*x_20 + x_14*x_20 + x_16*x_20 + x_17*x_20 + x_19*x_20 + x_2*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_9*x_21 + x_12*x_21 + x_13*x_21 + x_14*x_21 + x_16*x_21 + \n"
                       "        x_17*x_21 + x_19*x_21 + x_1*x_22 + x_3*x_22 + x_4*x_22 + x_9*x_22 + x_14*x_22 + x_17*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_4*x_23 + x_6*x_23 + x_8*x_23 + x_11*x_23 + x_12*x_23 + \n"
                       "        x_19*x_23 + x_21*x_23 + x_1*x_24 + x_2*x_24 + x_5*x_24 + x_6*x_24 + x_7*x_24 + x_8*x_24 + x_9*x_24 + x_11*x_24 + x_14*x_24 + x_16*x_24 + x_21*x_24 + x_22*x_24 + x_1*x_25 + x_2*x_25 + x_7*x_25 + \n"
                       "        x_8*x_25 + x_10*x_25 + x_13*x_25 + x_16*x_25 + x_17*x_25 + x_19*x_25 + x_21*x_25 + x_23*x_25 + x_24*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_4*x_26 + x_5*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + \n"
                       "        x_9*x_26 + x_10*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_16*x_26 + x_17*x_26 + x_18*x_26 + x_20*x_26 + x_23*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_6*x_27 + x_9*x_27 + x_12*x_27\n"
                       "        + x_13*x_27 + x_14*x_27 + x_15*x_27 + x_16*x_27 + x_17*x_27 + x_22*x_27 + x_24*x_27 + x_26*x_27 + x_1*x_28 + x_5*x_28 + x_10*x_28 + x_12*x_28 + x_13*x_28 + x_15*x_28 + x_17*x_28 + x_19*x_28 + \n"
                       "        x_21*x_28 + x_24*x_28 + x_26*x_28 + x_1*x_29 + x_2*x_29 + x_5*x_29 + x_8*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_13*x_29 + x_15*x_29 + x_18*x_29 + x_20*x_29 + x_21*x_29 + x_22*x_29 + x_23*x_29\n"
                       "        + x_24*x_29 + x_26*x_29 + x_27*x_29 + x_1*x_30 + x_4*x_30 + x_5*x_30 + x_7*x_30 + x_11*x_30 + x_13*x_30 + x_16*x_30 + x_17*x_30 + x_19*x_30 + x_22*x_30 + x_25*x_30 + x_26*x_30 + x_27*x_30 + \n"
                       "        x_28*x_30 + x_2*x_31 + x_3*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_8*x_31 + x_10*x_31 + x_14*x_31 + x_15*x_31 + x_16*x_31 + x_17*x_31 + x_18*x_31 + x_19*x_31 + x_21*x_31 + x_22*x_31 +\n"
                       "        x_23*x_31 + x_25*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_5*x_32 + x_6*x_32 + x_11*x_32 + x_14*x_32 + x_16*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 + x_23*x_32 + x_24*x_32 + x_25*x_32 + \n"
                       "        x_26*x_32 + x_28*x_32 + x_30*x_32 + x_31*x_32 + x_1*x_33 + x_5*x_33 + x_6*x_33 + x_7*x_33 + x_9*x_33 + x_13*x_33 + x_14*x_33 + x_17*x_33 + x_20*x_33 + x_21*x_33 + x_23*x_33 + x_24*x_33 + x_27*x_33\n"
                       "        + x_28*x_33 + x_31*x_33 + x_1*x_34 + x_4*x_34 + x_5*x_34 + x_6*x_34 + x_7*x_34 + x_9*x_34 + x_12*x_34 + x_14*x_34 + x_15*x_34 + x_17*x_34 + x_21*x_34 + x_22*x_34 + x_23*x_34 + x_24*x_34 + \n"
                       "        x_25*x_34 + x_26*x_34 + x_27*x_34 + x_28*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_2*x_35 + x_5*x_35 + x_6*x_35 + x_7*x_35 + x_10*x_35 + x_15*x_35 + x_19*x_35 + x_22*x_35 + \n"
                       "        x_23*x_35 + x_27*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_5*x_36 + x_7*x_36 + x_9*x_36 + x_10*x_36 + x_13*x_36 + x_15*x_36 + x_16*x_36 + x_17*x_36 + x_18*x_36 \n"
                       "        + x_19*x_36 + x_21*x_36 + x_23*x_36 + x_28*x_36 + x_30*x_36 + x_31*x_36 + x_32*x_36 + x_34*x_36 + x_1*x_37 + x_4*x_37 + x_5*x_37 + x_9*x_37 + x_10*x_37 + x_12*x_37 + x_13*x_37 + x_14*x_37 + \n"
                       "        x_16*x_37 + x_19*x_37 + x_21*x_37 + x_23*x_37 + x_27*x_37 + x_28*x_37 + x_31*x_37 + x_33*x_37 + x_36*x_37 + x_2*x_38 + x_3*x_38 + x_5*x_38 + x_8*x_38 + x_13*x_38 + x_14*x_38 + x_15*x_38 + \n"
                       "        x_18*x_38 + x_19*x_38 + x_21*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_1*x_39 + x_2*x_39 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_7*x_39 +\n"
                       "        x_8*x_39 + x_10*x_39 + x_11*x_39 + x_12*x_39 + x_14*x_39 + x_16*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_23*x_39 + x_24*x_39 + x_25*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_32*x_39 + \n"
                       "        x_33*x_39 + x_35*x_39 + x_37*x_39 + x_2*x_40 + x_7*x_40 + x_9*x_40 + x_11*x_40 + x_14*x_40 + x_15*x_40 + x_17*x_40 + x_18*x_40 + x_22*x_40 + x_24*x_40 + x_26*x_40 + x_27*x_40 + x_28*x_40 + \n"
                       "        x_30*x_40 + x_31*x_40 + x_32*x_40 + x_36*x_40 + x_37*x_40 + x_4*x_41 + x_5*x_41 + x_7*x_41 + x_8*x_41 + x_9*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_18*x_41\n"
                       "        + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_22*x_41 + x_24*x_41 + x_29*x_41 + x_30*x_41 + x_31*x_41 + x_33*x_41 + x_36*x_41 + x_38*x_41 + x_39*x_41 + x_40*x_41 + x_1*x_42 + x_6*x_42 + x_8*x_42 + \n"
                       "        x_9*x_42 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_14*x_42 + x_18*x_42 + x_20*x_42 + x_26*x_42 + x_27*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_34*x_42 + x_35*x_42 + x_36*x_42 + x_37*x_42 + \n"
                       "        x_38*x_42 + x_40*x_42 + x_41*x_42 + x_5*x_43 + x_7*x_43 + x_8*x_43 + x_10*x_43 + x_11*x_43 + x_13*x_43 + x_16*x_43 + x_18*x_43 + x_20*x_43 + x_22*x_43 + x_23*x_43 + x_24*x_43 + x_26*x_43 + \n"
                       "        x_33*x_43 + x_34*x_43 + x_37*x_43 + x_38*x_43 + x_3*x_44 + x_4*x_44 + x_6*x_44 + x_7*x_44 + x_8*x_44 + x_9*x_44 + x_11*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_22*x_44 + x_23*x_44 + x_24*x_44 \n"
                       "        + x_25*x_44 + x_26*x_44 + x_31*x_44 + x_33*x_44 + x_35*x_44 + x_36*x_44 + x_39*x_44 + x_40*x_44 + x_41*x_44 + x_1*x_45 + x_3*x_45 + x_4*x_45 + x_6*x_45 + x_10*x_45 + x_13*x_45 + x_14*x_45 + \n"
                       "        x_15*x_45 + x_21*x_45 + x_22*x_45 + x_24*x_45 + x_25*x_45 + x_27*x_45 + x_28*x_45 + x_31*x_45 + x_33*x_45 + x_40*x_45 + x_43*x_45 + x_44*x_45 + x_2*x_46 + x_6*x_46 + x_8*x_46 + x_10*x_46 + \n"
                       "        x_13*x_46 + x_15*x_46 + x_16*x_46 + x_18*x_46 + x_19*x_46 + x_20*x_46 + x_24*x_46 + x_27*x_46 + x_29*x_46 + x_32*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_39*x_46 + x_42*x_46 + \n"
                       "        x_44*x_46 + x_45*x_46 + x_1*x_47 + x_3*x_47 + x_6*x_47 + x_7*x_47 + x_9*x_47 + x_11*x_47 + x_13*x_47 + x_15*x_47 + x_16*x_47 + x_19*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_25*x_47 + x_26*x_47\n"
                       "        + x_27*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_38*x_47 + x_40*x_47 + x_45*x_47 + x_46*x_47 + x_6*x_48 + x_8*x_48 + x_9*x_48 + x_11*x_48 + x_15*x_48 + \n"
                       "        x_16*x_48 + x_22*x_48 + x_28*x_48 + x_31*x_48 + x_33*x_48 + x_36*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_43*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_1*x_49 + x_3*x_49 + x_4*x_49 + \n"
                       "        x_7*x_49 + x_9*x_49 + x_10*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_17*x_49 + x_19*x_49 + x_21*x_49 + x_22*x_49 + x_23*x_49 + x_26*x_49 + x_27*x_49 + x_29*x_49 + x_33*x_49 + \n"
                       "        x_36*x_49 + x_37*x_49 + x_39*x_49 + x_41*x_49 + x_42*x_49 + x_43*x_49 + x_44*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_4 + x_5 + x_6 + x_8 + x_9 + x_10 + x_12 + x_13 + x_14 + x_15 +\n"
                       "        x_16 + x_18 + x_19 + x_20 + x_21 + x_25 + x_26 + x_27 + x_32 + x_34 + x_35 + x_36 + x_41,\n"
                       "    x_1*x_2 + x_1*x_4 + x_3*x_4 + x_2*x_5 + x_4*x_6 + x_3*x_7 + x_5*x_7 + x_6*x_7 + x_3*x_8 + x_4*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + x_8*x_9 + x_1*x_10 + x_2*x_10 \n"
                       "        + x_5*x_10 + x_8*x_10 + x_2*x_11 + x_6*x_11 + x_7*x_11 + x_9*x_11 + x_1*x_12 + x_2*x_12 + x_4*x_12 + x_8*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_5*x_13 + x_6*x_13 + x_8*x_13 + x_9*x_13 + \n"
                       "        x_12*x_13 + x_1*x_14 + x_3*x_14 + x_6*x_14 + x_9*x_14 + x_10*x_14 + x_12*x_14 + x_13*x_14 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_8*x_15 + x_9*x_15 + x_10*x_15 + x_14*x_15 + x_2*x_16 + x_5*x_16 + \n"
                       "        x_6*x_16 + x_7*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_1*x_17 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_6*x_17 + x_10*x_17 + x_12*x_17 + x_13*x_17 + \n"
                       "        x_15*x_17 + x_16*x_17 + x_1*x_18 + x_4*x_18 + x_6*x_18 + x_7*x_18 + x_8*x_18 + x_13*x_18 + x_15*x_18 + x_16*x_18 + x_4*x_19 + x_7*x_19 + x_12*x_19 + x_13*x_19 + x_14*x_19 + x_16*x_19 + x_17*x_19 +\n"
                       "        x_1*x_20 + x_3*x_20 + x_4*x_20 + x_7*x_20 + x_8*x_20 + x_9*x_20 + x_16*x_20 + x_17*x_20 + x_18*x_20 + x_19*x_20 + x_2*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_9*x_21 + x_14*x_21 + x_19*x_21 + \n"
                       "        x_20*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_4*x_22 + x_7*x_22 + x_9*x_22 + x_13*x_22 + x_15*x_22 + x_19*x_22 + x_21*x_22 + x_2*x_23 + x_4*x_23 + x_5*x_23 + x_8*x_23 + x_9*x_23 + x_10*x_23 + \n"
                       "        x_11*x_23 + x_13*x_23 + x_14*x_23 + x_15*x_23 + x_16*x_23 + x_18*x_23 + x_22*x_23 + x_1*x_24 + x_5*x_24 + x_6*x_24 + x_8*x_24 + x_10*x_24 + x_13*x_24 + x_14*x_24 + x_16*x_24 + x_18*x_24 + \n"
                       "        x_21*x_24 + x_22*x_24 + x_2*x_25 + x_3*x_25 + x_4*x_25 + x_6*x_25 + x_8*x_25 + x_11*x_25 + x_12*x_25 + x_13*x_25 + x_15*x_25 + x_17*x_25 + x_20*x_25 + x_22*x_25 + x_1*x_26 + x_2*x_26 + x_4*x_26 + \n"
                       "        x_5*x_26 + x_7*x_26 + x_8*x_26 + x_9*x_26 + x_10*x_26 + x_16*x_26 + x_17*x_26 + x_18*x_26 + x_19*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_2*x_27 + x_3*x_27 + x_4*x_27 + x_6*x_27 + x_7*x_27 + \n"
                       "        x_9*x_27 + x_12*x_27 + x_14*x_27 + x_15*x_27 + x_17*x_27 + x_20*x_27 + x_22*x_27 + x_24*x_27 + x_26*x_27 + x_1*x_28 + x_2*x_28 + x_5*x_28 + x_7*x_28 + x_9*x_28 + x_10*x_28 + x_11*x_28 + x_12*x_28 \n"
                       "        + x_13*x_28 + x_15*x_28 + x_18*x_28 + x_23*x_28 + x_24*x_28 + x_25*x_28 + x_26*x_28 + x_27*x_28 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_7*x_29 + x_8*x_29 + x_9*x_29 + x_10*x_29 + x_18*x_29 + \n"
                       "        x_20*x_29 + x_21*x_29 + x_25*x_29 + x_28*x_29 + x_3*x_30 + x_4*x_30 + x_7*x_30 + x_8*x_30 + x_9*x_30 + x_10*x_30 + x_12*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_17*x_30 + x_20*x_30 + x_21*x_30\n"
                       "        + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_28*x_30 + x_29*x_30 + x_1*x_31 + x_2*x_31 + x_3*x_31 + x_8*x_31 + x_9*x_31 + x_12*x_31 + x_14*x_31 + x_16*x_31 + x_17*x_31 + x_19*x_31 + x_20*x_31 + \n"
                       "        x_27*x_31 + x_29*x_31 + x_30*x_31 + x_7*x_32 + x_8*x_32 + x_9*x_32 + x_17*x_32 + x_18*x_32 + x_21*x_32 + x_22*x_32 + x_25*x_32 + x_26*x_32 + x_28*x_32 + x_29*x_32 + x_31*x_32 + x_1*x_33 + x_2*x_33\n"
                       "        + x_4*x_33 + x_5*x_33 + x_14*x_33 + x_15*x_33 + x_16*x_33 + x_19*x_33 + x_22*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_32*x_33 + x_2*x_34 + x_3*x_34 + x_4*x_34 + x_8*x_34 + x_9*x_34\n"
                       "        + x_11*x_34 + x_12*x_34 + x_14*x_34 + x_16*x_34 + x_17*x_34 + x_19*x_34 + x_21*x_34 + x_22*x_34 + x_23*x_34 + x_24*x_34 + x_25*x_34 + x_27*x_34 + x_28*x_34 + x_29*x_34 + x_30*x_34 + x_32*x_34 + \n"
                       "        x_33*x_34 + x_1*x_35 + x_5*x_35 + x_9*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_15*x_35 + x_20*x_35 + x_26*x_35 + x_27*x_35 + x_29*x_35 + x_32*x_35 + x_1*x_36 + x_2*x_36 + x_5*x_36 + x_7*x_36 +\n"
                       "        x_8*x_36 + x_9*x_36 + x_10*x_36 + x_11*x_36 + x_18*x_36 + x_23*x_36 + x_26*x_36 + x_27*x_36 + x_32*x_36 + x_33*x_36 + x_35*x_36 + x_1*x_37 + x_2*x_37 + x_7*x_37 + x_9*x_37 + x_10*x_37 + x_12*x_37 \n"
                       "        + x_15*x_37 + x_17*x_37 + x_21*x_37 + x_22*x_37 + x_23*x_37 + x_24*x_37 + x_26*x_37 + x_27*x_37 + x_30*x_37 + x_32*x_37 + x_33*x_37 + x_34*x_37 + x_1*x_38 + x_4*x_38 + x_5*x_38 + x_7*x_38 + \n"
                       "        x_11*x_38 + x_12*x_38 + x_13*x_38 + x_14*x_38 + x_16*x_38 + x_17*x_38 + x_19*x_38 + x_21*x_38 + x_24*x_38 + x_28*x_38 + x_29*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_34*x_38 + x_35*x_38 + \n"
                       "        x_36*x_38 + x_1*x_39 + x_2*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_10*x_39 + x_13*x_39 + x_21*x_39 + x_23*x_39 + x_25*x_39 + x_26*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_33*x_39 + x_37*x_39\n"
                       "        + x_38*x_39 + x_1*x_40 + x_3*x_40 + x_4*x_40 + x_8*x_40 + x_9*x_40 + x_10*x_40 + x_12*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_20*x_40 + x_28*x_40 + x_29*x_40 + x_34*x_40 + x_35*x_40 + \n"
                       "        x_36*x_40 + x_1*x_41 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_10*x_41 + x_13*x_41 + x_14*x_41 + x_16*x_41 + x_17*x_41 + x_20*x_41 + x_21*x_41 + x_27*x_41 + x_28*x_41 + x_29*x_41 + x_32*x_41 + \n"
                       "        x_35*x_41 + x_38*x_41 + x_39*x_41 + x_40*x_41 + x_1*x_42 + x_2*x_42 + x_3*x_42 + x_5*x_42 + x_7*x_42 + x_8*x_42 + x_10*x_42 + x_11*x_42 + x_14*x_42 + x_16*x_42 + x_17*x_42 + x_21*x_42 + x_22*x_42 \n"
                       "        + x_23*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_28*x_42 + x_29*x_42 + x_30*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_35*x_42 + x_38*x_42 + x_40*x_42 + x_4*x_43 + x_5*x_43 + x_8*x_43 + \n"
                       "        x_10*x_43 + x_11*x_43 + x_14*x_43 + x_15*x_43 + x_19*x_43 + x_22*x_43 + x_23*x_43 + x_25*x_43 + x_28*x_43 + x_29*x_43 + x_30*x_43 + x_33*x_43 + x_34*x_43 + x_35*x_43 + x_38*x_43 + x_41*x_43 + \n"
                       "        x_42*x_43 + x_4*x_44 + x_7*x_44 + x_8*x_44 + x_9*x_44 + x_10*x_44 + x_11*x_44 + x_16*x_44 + x_22*x_44 + x_24*x_44 + x_27*x_44 + x_28*x_44 + x_29*x_44 + x_32*x_44 + x_36*x_44 + x_37*x_44 + \n"
                       "        x_39*x_44 + x_41*x_44 + x_42*x_44 + x_43*x_44 + x_1*x_45 + x_3*x_45 + x_4*x_45 + x_7*x_45 + x_13*x_45 + x_16*x_45 + x_18*x_45 + x_19*x_45 + x_20*x_45 + x_21*x_45 + x_23*x_45 + x_24*x_45 + \n"
                       "        x_25*x_45 + x_27*x_45 + x_30*x_45 + x_31*x_45 + x_32*x_45 + x_33*x_45 + x_35*x_45 + x_37*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_1*x_46 + x_2*x_46 + x_4*x_46 + x_5*x_46 + x_9*x_46 + x_10*x_46\n"
                       "        + x_14*x_46 + x_15*x_46 + x_16*x_46 + x_18*x_46 + x_19*x_46 + x_20*x_46 + x_25*x_46 + x_26*x_46 + x_32*x_46 + x_35*x_46 + x_37*x_46 + x_38*x_46 + x_41*x_46 + x_42*x_46 + x_6*x_47 + x_11*x_47 + \n"
                       "        x_13*x_47 + x_16*x_47 + x_20*x_47 + x_21*x_47 + x_26*x_47 + x_27*x_47 + x_29*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_41*x_47 + x_43*x_47 + x_2*x_48 + x_3*x_48 + x_4*x_48 + \n"
                       "        x_8*x_48 + x_9*x_48 + x_10*x_48 + x_12*x_48 + x_18*x_48 + x_20*x_48 + x_26*x_48 + x_29*x_48 + x_30*x_48 + x_31*x_48 + x_32*x_48 + x_35*x_48 + x_36*x_48 + x_37*x_48 + x_38*x_48 + x_39*x_48 + \n"
                       "        x_40*x_48 + x_41*x_48 + x_44*x_48 + x_45*x_48 + x_47*x_48 + x_2*x_49 + x_3*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_12*x_49 + x_13*x_49 + x_15*x_49 + x_16*x_49 + x_19*x_49 + x_20*x_49 + x_21*x_49\n"
                       "        + x_22*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + x_38*x_49 + x_40*x_49 + x_41*x_49 + x_43*x_49 + x_47*x_49 + \n"
                       "        x_3 + x_4 + x_5 + x_8 + x_12 + x_13 + x_15 + x_16 + x_17 + x_18 + x_20 + x_21 + x_22 + x_28 + x_30 + x_33 + x_34 + x_35 + x_36 + x_37 + x_38 + x_39 + x_41 + x_48 + x_49,\n"
                       "    x_2*x_3 + x_1*x_4 + x_3*x_4 + x_1*x_5 + x_3*x_5 + x_3*x_6 + x_4*x_6 + x_1*x_7 + x_3*x_7 + x_4*x_7 + x_5*x_7 + x_6*x_7 + x_3*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_5*x_9 + x_8*x_9 + x_1*x_10 +\n"
                       "        x_7*x_10 + x_2*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_7*x_11 + x_9*x_11 + x_2*x_12 + x_4*x_12 + x_9*x_12 + x_10*x_12 + x_11*x_12 + x_2*x_13 + x_3*x_13 + x_4*x_13 + x_6*x_13 + x_7*x_13 + \n"
                       "        x_8*x_13 + x_10*x_13 + x_11*x_13 + x_1*x_14 + x_2*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14 + x_10*x_14 + x_2*x_15 + x_5*x_15 + x_6*x_15 + x_8*x_15 + x_11*x_15 + x_12*x_15 + x_13*x_15 + x_3*x_16 + \n"
                       "        x_4*x_16 + x_5*x_16 + x_6*x_16 + x_7*x_16 + x_8*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_6*x_17 + x_10*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + x_2*x_18 + x_3*x_18 + x_5*x_18 + \n"
                       "        x_6*x_18 + x_7*x_18 + x_10*x_18 + x_12*x_18 + x_13*x_18 + x_15*x_18 + x_17*x_18 + x_5*x_19 + x_6*x_19 + x_9*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_14*x_19 + x_16*x_19 + x_17*x_19 + x_4*x_20 \n"
                       "        + x_5*x_20 + x_6*x_20 + x_8*x_20 + x_10*x_20 + x_11*x_20 + x_13*x_20 + x_14*x_20 + x_15*x_20 + x_16*x_20 + x_1*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_11*x_21 + x_13*x_21 + x_19*x_21 + x_1*x_22 \n"
                       "        + x_3*x_22 + x_4*x_22 + x_6*x_22 + x_7*x_22 + x_12*x_22 + x_14*x_22 + x_15*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_2*x_23 + x_4*x_23 + x_7*x_23 + x_8*x_23 + x_9*x_23 + x_10*x_23 + x_11*x_23 +\n"
                       "        x_13*x_23 + x_14*x_23 + x_17*x_23 + x_19*x_23 + x_20*x_23 + x_22*x_23 + x_1*x_24 + x_2*x_24 + x_5*x_24 + x_6*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_12*x_24 + x_13*x_24 + x_15*x_24 + x_16*x_24 \n"
                       "        + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_1*x_25 + x_4*x_25 + x_5*x_25 + x_6*x_25 + x_9*x_25 + x_10*x_25 + x_11*x_25 + x_13*x_25 + x_18*x_25 + x_20*x_25 + x_21*x_25 + x_24*x_25 + x_2*x_26 + x_7*x_26\n"
                       "        + x_8*x_26 + x_9*x_26 + x_10*x_26 + x_13*x_26 + x_16*x_26 + x_17*x_26 + x_23*x_26 + x_24*x_26 + x_2*x_27 + x_3*x_27 + x_7*x_27 + x_9*x_27 + x_17*x_27 + x_19*x_27 + x_22*x_27 + x_23*x_27 + x_2*x_28\n"
                       "        + x_3*x_28 + x_6*x_28 + x_7*x_28 + x_11*x_28 + x_15*x_28 + x_17*x_28 + x_19*x_28 + x_23*x_28 + x_24*x_28 + x_4*x_29 + x_5*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_12*x_29 + x_13*x_29 + \n"
                       "        x_17*x_29 + x_19*x_29 + x_20*x_29 + x_24*x_29 + x_25*x_29 + x_26*x_29 + x_27*x_29 + x_28*x_29 + x_1*x_30 + x_2*x_30 + x_5*x_30 + x_6*x_30 + x_8*x_30 + x_9*x_30 + x_10*x_30 + x_14*x_30 + x_15*x_30 \n"
                       "        + x_17*x_30 + x_18*x_30 + x_19*x_30 + x_22*x_30 + x_23*x_30 + x_24*x_30 + x_25*x_30 + x_28*x_30 + x_29*x_30 + x_2*x_31 + x_3*x_31 + x_4*x_31 + x_7*x_31 + x_8*x_31 + x_17*x_31 + x_18*x_31 + \n"
                       "        x_20*x_31 + x_21*x_31 + x_24*x_31 + x_25*x_31 + x_27*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_3*x_32 + x_5*x_32 + x_6*x_32 + x_8*x_32 + x_10*x_32 + x_13*x_32 + x_17*x_32 + x_18*x_32 + \n"
                       "        x_19*x_32 + x_20*x_32 + x_22*x_32 + x_24*x_32 + x_25*x_32 + x_28*x_32 + x_31*x_32 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_5*x_33 + x_7*x_33 + x_8*x_33 + x_9*x_33 + x_10*x_33 + x_12*x_33 + x_13*x_33 +\n"
                       "        x_14*x_33 + x_16*x_33 + x_17*x_33 + x_18*x_33 + x_19*x_33 + x_24*x_33 + x_25*x_33 + x_26*x_33 + x_29*x_33 + x_30*x_33 + x_31*x_33 + x_1*x_34 + x_2*x_34 + x_4*x_34 + x_6*x_34 + x_7*x_34 + x_8*x_34 \n"
                       "        + x_10*x_34 + x_11*x_34 + x_13*x_34 + x_14*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_21*x_34 + x_27*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_1*x_35 + x_2*x_35 + x_3*x_35 + x_5*x_35 + \n"
                       "        x_6*x_35 + x_11*x_35 + x_13*x_35 + x_14*x_35 + x_17*x_35 + x_18*x_35 + x_22*x_35 + x_27*x_35 + x_29*x_35 + x_30*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_7*x_36 + x_8*x_36 + x_15*x_36 \n"
                       "        + x_16*x_36 + x_18*x_36 + x_19*x_36 + x_21*x_36 + x_22*x_36 + x_24*x_36 + x_26*x_36 + x_32*x_36 + x_34*x_36 + x_1*x_37 + x_5*x_37 + x_6*x_37 + x_9*x_37 + x_11*x_37 + x_15*x_37 + x_16*x_37 + \n"
                       "        x_17*x_37 + x_18*x_37 + x_19*x_37 + x_20*x_37 + x_21*x_37 + x_23*x_37 + x_24*x_37 + x_25*x_37 + x_27*x_37 + x_28*x_37 + x_30*x_37 + x_32*x_37 + x_35*x_37 + x_36*x_37 + x_3*x_38 + x_4*x_38 + \n"
                       "        x_5*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_10*x_38 + x_14*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_18*x_38 + x_19*x_38 + x_23*x_38 + x_24*x_38 + x_27*x_38 + x_28*x_38 + x_32*x_38 + \n"
                       "        x_35*x_38 + x_36*x_38 + x_1*x_39 + x_7*x_39 + x_11*x_39 + x_12*x_39 + x_15*x_39 + x_19*x_39 + x_20*x_39 + x_21*x_39 + x_25*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_32*x_39 + x_33*x_39 + \n"
                       "        x_34*x_39 + x_35*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 + x_5*x_40 + x_7*x_40 + x_13*x_40 + x_21*x_40 + x_23*x_40 + x_24*x_40 + x_26*x_40 + x_28*x_40 + x_31*x_40 + x_34*x_40 + x_36*x_40 + \n"
                       "        x_38*x_40 + x_5*x_41 + x_6*x_41 + x_7*x_41 + x_8*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_16*x_41 + x_17*x_41 + x_19*x_41 + x_21*x_41 + x_23*x_41 + x_24*x_41 + \n"
                       "        x_25*x_41 + x_26*x_41 + x_27*x_41 + x_28*x_41 + x_29*x_41 + x_34*x_41 + x_35*x_41 + x_36*x_41 + x_38*x_41 + x_39*x_41 + x_40*x_41 + x_2*x_42 + x_3*x_42 + x_8*x_42 + x_9*x_42 + x_12*x_42 + \n"
                       "        x_13*x_42 + x_14*x_42 + x_15*x_42 + x_17*x_42 + x_20*x_42 + x_21*x_42 + x_23*x_42 + x_25*x_42 + x_26*x_42 + x_27*x_42 + x_30*x_42 + x_31*x_42 + x_32*x_42 + x_33*x_42 + x_38*x_42 + x_39*x_42 + \n"
                       "        x_40*x_42 + x_1*x_43 + x_3*x_43 + x_4*x_43 + x_9*x_43 + x_11*x_43 + x_17*x_43 + x_18*x_43 + x_21*x_43 + x_26*x_43 + x_27*x_43 + x_32*x_43 + x_35*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + \n"
                       "        x_42*x_43 + x_2*x_44 + x_3*x_44 + x_5*x_44 + x_6*x_44 + x_8*x_44 + x_9*x_44 + x_11*x_44 + x_15*x_44 + x_16*x_44 + x_19*x_44 + x_21*x_44 + x_28*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_37*x_44 \n"
                       "        + x_40*x_44 + x_41*x_44 + x_1*x_45 + x_2*x_45 + x_5*x_45 + x_8*x_45 + x_9*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_14*x_45 + x_15*x_45 + x_16*x_45 + x_17*x_45 + x_19*x_45 + x_26*x_45 + \n"
                       "        x_28*x_45 + x_30*x_45 + x_33*x_45 + x_34*x_45 + x_35*x_45 + x_37*x_45 + x_40*x_45 + x_41*x_45 + x_42*x_45 + x_44*x_45 + x_4*x_46 + x_5*x_46 + x_6*x_46 + x_7*x_46 + x_9*x_46 + x_11*x_46 + x_13*x_46\n"
                       "        + x_14*x_46 + x_15*x_46 + x_20*x_46 + x_21*x_46 + x_23*x_46 + x_24*x_46 + x_25*x_46 + x_26*x_46 + x_27*x_46 + x_31*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_36*x_46 + x_38*x_46 + x_39*x_46 + \n"
                       "        x_41*x_46 + x_42*x_46 + x_45*x_46 + x_4*x_47 + x_6*x_47 + x_7*x_47 + x_9*x_47 + x_13*x_47 + x_14*x_47 + x_17*x_47 + x_18*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_24*x_47 + x_26*x_47 + \n"
                       "        x_29*x_47 + x_30*x_47 + x_32*x_47 + x_34*x_47 + x_35*x_47 + x_39*x_47 + x_41*x_47 + x_43*x_47 + x_44*x_47 + x_2*x_48 + x_3*x_48 + x_4*x_48 + x_6*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + \n"
                       "        x_14*x_48 + x_17*x_48 + x_18*x_48 + x_19*x_48 + x_20*x_48 + x_21*x_48 + x_22*x_48 + x_23*x_48 + x_24*x_48 + x_25*x_48 + x_26*x_48 + x_27*x_48 + x_28*x_48 + x_31*x_48 + x_33*x_48 + x_38*x_48 + \n"
                       "        x_39*x_48 + x_40*x_48 + x_44*x_48 + x_47*x_48 + x_1*x_49 + x_2*x_49 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_9*x_49 + x_10*x_49 + x_14*x_49 + x_16*x_49 + x_18*x_49 + x_20*x_49 + x_23*x_49 +\n"
                       "        x_25*x_49 + x_26*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_33*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + x_40*x_49 + x_43*x_49 + x_44*x_49 + x_46*x_49 + x_47*x_49 + x_3 + x_5 + x_8 \n"
                       "        + x_9 + x_11 + x_12 + x_13 + x_15 + x_19 + x_20 + x_21 + x_23 + x_24 + x_26 + x_29 + x_30 + x_31 + x_32 + x_33 + x_34 + x_35 + x_37 + x_40 + x_43 + x_45 + x_49,\n"
                       "    x_2*x_3 + x_1*x_4 + x_2*x_4 + x_3*x_4 + x_1*x_5 + x_3*x_5 + x_4*x_5 + x_3*x_6 + x_4*x_6 + x_5*x_6 + x_1*x_7 + x_2*x_7 + x_5*x_7 + x_3*x_8 + x_6*x_8 + x_2*x_9 + x_3*x_9 + x_7*x_9 + x_8*x_9 + x_1*x_10 +\n"
                       "        x_2*x_10 + x_4*x_10 + x_5*x_10 + x_9*x_10 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_7*x_11 + x_8*x_11 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_9*x_12 + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_5*x_13 + x_7*x_13\n"
                       "        + x_12*x_13 + x_1*x_14 + x_4*x_14 + x_5*x_14 + x_6*x_14 + x_7*x_14 + x_8*x_14 + x_11*x_14 + x_2*x_15 + x_3*x_15 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_7*x_15 + x_8*x_15 + x_11*x_15 + x_14*x_15 + \n"
                       "        x_1*x_16 + x_5*x_16 + x_8*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_1*x_17 + x_3*x_17 + x_4*x_17 + x_9*x_17 + x_11*x_17 + x_12*x_17 + x_13*x_17 + x_14*x_17 + x_15*x_17 + \n"
                       "        x_16*x_17 + x_1*x_18 + x_2*x_18 + x_5*x_18 + x_6*x_18 + x_7*x_18 + x_11*x_18 + x_13*x_18 + x_14*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + x_2*x_19 + x_4*x_19 + x_7*x_19 + x_8*x_19 + x_12*x_19 + \n"
                       "        x_14*x_19 + x_15*x_19 + x_17*x_19 + x_4*x_20 + x_6*x_20 + x_8*x_20 + x_10*x_20 + x_11*x_20 + x_12*x_20 + x_15*x_20 + x_16*x_20 + x_19*x_20 + x_1*x_21 + x_2*x_21 + x_4*x_21 + x_11*x_21 + x_12*x_21 \n"
                       "        + x_14*x_21 + x_17*x_21 + x_20*x_21 + x_2*x_22 + x_3*x_22 + x_5*x_22 + x_7*x_22 + x_8*x_22 + x_10*x_22 + x_13*x_22 + x_15*x_22 + x_17*x_22 + x_1*x_23 + x_2*x_23 + x_3*x_23 + x_4*x_23 + x_8*x_23 + \n"
                       "        x_9*x_23 + x_10*x_23 + x_12*x_23 + x_19*x_23 + x_20*x_23 + x_21*x_23 + x_5*x_24 + x_7*x_24 + x_10*x_24 + x_12*x_24 + x_14*x_24 + x_16*x_24 + x_17*x_24 + x_18*x_24 + x_20*x_24 + x_4*x_25 + x_5*x_25\n"
                       "        + x_8*x_25 + x_9*x_25 + x_12*x_25 + x_13*x_25 + x_15*x_25 + x_18*x_25 + x_20*x_25 + x_21*x_25 + x_22*x_25 + x_23*x_25 + x_1*x_26 + x_9*x_26 + x_10*x_26 + x_13*x_26 + x_14*x_26 + x_16*x_26 + \n"
                       "        x_17*x_26 + x_18*x_26 + x_20*x_26 + x_22*x_26 + x_24*x_26 + x_3*x_27 + x_5*x_27 + x_6*x_27 + x_8*x_27 + x_10*x_27 + x_11*x_27 + x_12*x_27 + x_19*x_27 + x_20*x_27 + x_21*x_27 + x_22*x_27 + \n"
                       "        x_23*x_27 + x_24*x_27 + x_25*x_27 + x_26*x_27 + x_2*x_28 + x_4*x_28 + x_9*x_28 + x_10*x_28 + x_12*x_28 + x_13*x_28 + x_15*x_28 + x_20*x_28 + x_21*x_28 + x_25*x_28 + x_27*x_28 + x_1*x_29 + x_3*x_29\n"
                       "        + x_5*x_29 + x_7*x_29 + x_9*x_29 + x_10*x_29 + x_11*x_29 + x_12*x_29 + x_15*x_29 + x_16*x_29 + x_17*x_29 + x_18*x_29 + x_21*x_29 + x_23*x_29 + x_24*x_29 + x_1*x_30 + x_5*x_30 + x_7*x_30 + \n"
                       "        x_10*x_30 + x_14*x_30 + x_17*x_30 + x_18*x_30 + x_19*x_30 + x_21*x_30 + x_22*x_30 + x_23*x_30 + x_26*x_30 + x_27*x_30 + x_28*x_30 + x_29*x_30 + x_2*x_31 + x_4*x_31 + x_5*x_31 + x_7*x_31 + x_8*x_31\n"
                       "        + x_9*x_31 + x_10*x_31 + x_12*x_31 + x_14*x_31 + x_19*x_31 + x_20*x_31 + x_21*x_31 + x_26*x_31 + x_30*x_31 + x_4*x_32 + x_9*x_32 + x_11*x_32 + x_14*x_32 + x_15*x_32 + x_16*x_32 + x_18*x_32 + \n"
                       "        x_20*x_32 + x_21*x_32 + x_22*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_31*x_32 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_5*x_33 + x_13*x_33 + x_15*x_33 + x_16*x_33 + x_17*x_33 + \n"
                       "        x_18*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_24*x_33 + x_26*x_33 + x_27*x_33 + x_32*x_33 + x_6*x_34 + x_10*x_34 + x_11*x_34 + x_13*x_34 + x_15*x_34 + x_16*x_34 + x_17*x_34 + \n"
                       "        x_18*x_34 + x_19*x_34 + x_22*x_34 + x_27*x_34 + x_29*x_34 + x_32*x_34 + x_1*x_35 + x_4*x_35 + x_9*x_35 + x_10*x_35 + x_14*x_35 + x_15*x_35 + x_16*x_35 + x_18*x_35 + x_19*x_35 + x_21*x_35 + \n"
                       "        x_24*x_35 + x_25*x_35 + x_27*x_35 + x_29*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_4*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 + x_13*x_36 + x_14*x_36 + x_16*x_36 \n"
                       "        + x_20*x_36 + x_22*x_36 + x_26*x_36 + x_27*x_36 + x_28*x_36 + x_29*x_36 + x_30*x_36 + x_31*x_36 + x_33*x_36 + x_35*x_36 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_5*x_37 + x_7*x_37 + x_10*x_37 + \n"
                       "        x_12*x_37 + x_13*x_37 + x_15*x_37 + x_19*x_37 + x_22*x_37 + x_23*x_37 + x_25*x_37 + x_27*x_37 + x_28*x_37 + x_29*x_37 + x_31*x_37 + x_33*x_37 + x_34*x_37 + x_36*x_37 + x_4*x_38 + x_8*x_38 + \n"
                       "        x_9*x_38 + x_11*x_38 + x_15*x_38 + x_18*x_38 + x_21*x_38 + x_23*x_38 + x_24*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_32*x_38 + x_34*x_38 + x_36*x_38 + x_37*x_38 + \n"
                       "        x_1*x_39 + x_4*x_39 + x_6*x_39 + x_7*x_39 + x_9*x_39 + x_13*x_39 + x_15*x_39 + x_16*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + x_25*x_39 + x_27*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39\n"
                       "        + x_34*x_39 + x_35*x_39 + x_3*x_40 + x_4*x_40 + x_7*x_40 + x_9*x_40 + x_10*x_40 + x_11*x_40 + x_12*x_40 + x_13*x_40 + x_14*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_18*x_40 + x_22*x_40 + \n"
                       "        x_23*x_40 + x_24*x_40 + x_27*x_40 + x_29*x_40 + x_33*x_40 + x_38*x_40 + x_39*x_40 + x_2*x_41 + x_3*x_41 + x_6*x_41 + x_16*x_41 + x_17*x_41 + x_20*x_41 + x_21*x_41 + x_23*x_41 + x_24*x_41 + \n"
                       "        x_25*x_41 + x_27*x_41 + x_30*x_41 + x_31*x_41 + x_35*x_41 + x_2*x_42 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_8*x_42 + x_10*x_42 + x_11*x_42 + x_12*x_42 + x_14*x_42 + x_15*x_42 +\n"
                       "        x_17*x_42 + x_18*x_42 + x_20*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_27*x_42 + x_32*x_42 + x_39*x_42 + x_40*x_42 + x_3*x_43 + x_7*x_43 + x_8*x_43 + x_12*x_43 + x_15*x_43 + x_16*x_43 + \n"
                       "        x_17*x_43 + x_18*x_43 + x_19*x_43 + x_20*x_43 + x_21*x_43 + x_22*x_43 + x_23*x_43 + x_27*x_43 + x_28*x_43 + x_29*x_43 + x_31*x_43 + x_32*x_43 + x_38*x_43 + x_39*x_43 + x_40*x_43 + x_41*x_43 + \n"
                       "        x_1*x_44 + x_4*x_44 + x_5*x_44 + x_6*x_44 + x_8*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_20*x_44 + x_21*x_44 + x_22*x_44 + x_23*x_44\n"
                       "        + x_24*x_44 + x_27*x_44 + x_31*x_44 + x_38*x_44 + x_39*x_44 + x_40*x_44 + x_41*x_44 + x_2*x_45 + x_4*x_45 + x_6*x_45 + x_8*x_45 + x_12*x_45 + x_13*x_45 + x_15*x_45 + x_17*x_45 + x_18*x_45 + \n"
                       "        x_19*x_45 + x_23*x_45 + x_24*x_45 + x_25*x_45 + x_26*x_45 + x_27*x_45 + x_31*x_45 + x_34*x_45 + x_35*x_45 + x_37*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_42*x_45 + x_6*x_46 + x_7*x_46 + \n"
                       "        x_8*x_46 + x_9*x_46 + x_10*x_46 + x_12*x_46 + x_13*x_46 + x_17*x_46 + x_19*x_46 + x_21*x_46 + x_22*x_46 + x_24*x_46 + x_26*x_46 + x_30*x_46 + x_34*x_46 + x_36*x_46 + x_38*x_46 + x_39*x_46 + \n"
                       "        x_40*x_46 + x_42*x_46 + x_43*x_46 + x_44*x_46 + x_45*x_46 + x_2*x_47 + x_3*x_47 + x_6*x_47 + x_10*x_47 + x_12*x_47 + x_14*x_47 + x_16*x_47 + x_17*x_47 + x_18*x_47 + x_21*x_47 + x_22*x_47 + \n"
                       "        x_23*x_47 + x_26*x_47 + x_31*x_47 + x_32*x_47 + x_35*x_47 + x_38*x_47 + x_39*x_47 + x_40*x_47 + x_46*x_47 + x_3*x_48 + x_4*x_48 + x_16*x_48 + x_18*x_48 + x_20*x_48 + x_22*x_48 + x_23*x_48 + \n"
                       "        x_24*x_48 + x_26*x_48 + x_27*x_48 + x_29*x_48 + x_30*x_48 + x_31*x_48 + x_32*x_48 + x_33*x_48 + x_35*x_48 + x_38*x_48 + x_39*x_48 + x_40*x_48 + x_41*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + \n"
                       "        x_47*x_48 + x_3*x_49 + x_4*x_49 + x_6*x_49 + x_14*x_49 + x_16*x_49 + x_17*x_49 + x_22*x_49 + x_23*x_49 + x_24*x_49 + x_30*x_49 + x_33*x_49 + x_36*x_49 + x_37*x_49 + x_41*x_49 + x_45*x_49 + \n"
                       "        x_46*x_49 + x_4 + x_5 + x_7 + x_12 + x_14 + x_19 + x_20 + x_23 + x_25 + x_29 + x_31 + x_33 + x_36 + x_37 + x_38 + x_39 + x_40 + x_41 + x_42 + x_45 + x_46,\n"
                       "    x_2*x_3 + x_2*x_4 + x_3*x_4 + x_1*x_5 + x_4*x_5 + x_1*x_6 + x_2*x_6 + x_5*x_6 + x_1*x_7 + x_5*x_7 + x_1*x_8 + x_3*x_8 + x_4*x_8 + x_6*x_8 + x_1*x_9 + x_2*x_9 + x_3*x_9 + x_4*x_9 + x_5*x_9 + x_6*x_9 + \n"
                       "        x_8*x_9 + x_1*x_10 + x_2*x_10 + x_4*x_10 + x_6*x_10 + x_9*x_10 + x_2*x_11 + x_3*x_11 + x_7*x_11 + x_9*x_11 + x_10*x_11 + x_6*x_12 + x_8*x_12 + x_9*x_12 + x_11*x_12 + x_7*x_13 + x_1*x_14 + x_3*x_14\n"
                       "        + x_5*x_14 + x_8*x_14 + x_9*x_14 + x_10*x_14 + x_13*x_14 + x_1*x_15 + x_2*x_15 + x_6*x_15 + x_7*x_15 + x_8*x_15 + x_12*x_15 + x_14*x_15 + x_1*x_16 + x_2*x_16 + x_7*x_16 + x_9*x_16 + x_10*x_16 + \n"
                       "        x_11*x_16 + x_12*x_16 + x_14*x_16 + x_15*x_16 + x_1*x_17 + x_2*x_17 + x_3*x_17 + x_5*x_17 + x_11*x_17 + x_14*x_17 + x_15*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_4*x_18 + x_5*x_18 + x_7*x_18 + \n"
                       "        x_8*x_18 + x_10*x_18 + x_11*x_18 + x_12*x_18 + x_16*x_18 + x_6*x_19 + x_7*x_19 + x_9*x_19 + x_14*x_19 + x_16*x_19 + x_18*x_19 + x_1*x_20 + x_4*x_20 + x_6*x_20 + x_7*x_20 + x_9*x_20 + x_10*x_20 + \n"
                       "        x_11*x_20 + x_12*x_20 + x_18*x_20 + x_19*x_20 + x_2*x_21 + x_5*x_21 + x_6*x_21 + x_8*x_21 + x_10*x_21 + x_11*x_21 + x_16*x_21 + x_17*x_21 + x_18*x_21 + x_19*x_21 + x_2*x_22 + x_4*x_22 + x_11*x_22 \n"
                       "        + x_12*x_22 + x_13*x_22 + x_18*x_22 + x_2*x_23 + x_3*x_23 + x_5*x_23 + x_8*x_23 + x_9*x_23 + x_10*x_23 + x_11*x_23 + x_13*x_23 + x_16*x_23 + x_20*x_23 + x_22*x_23 + x_5*x_24 + x_6*x_24 + x_7*x_24 \n"
                       "        + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_12*x_24 + x_13*x_24 + x_14*x_24 + x_15*x_24 + x_16*x_24 + x_17*x_24 + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_22*x_24 + x_23*x_24 + \n"
                       "        x_1*x_25 + x_3*x_25 + x_6*x_25 + x_7*x_25 + x_9*x_25 + x_11*x_25 + x_14*x_25 + x_16*x_25 + x_20*x_25 + x_21*x_25 + x_1*x_26 + x_2*x_26 + x_3*x_26 + x_10*x_26 + x_11*x_26 + x_14*x_26 + x_16*x_26 + \n"
                       "        x_18*x_26 + x_20*x_26 + x_22*x_26 + x_23*x_26 + x_25*x_26 + x_5*x_27 + x_7*x_27 + x_8*x_27 + x_9*x_27 + x_11*x_27 + x_12*x_27 + x_13*x_27 + x_14*x_27 + x_15*x_27 + x_22*x_27 + x_23*x_27 + \n"
                       "        x_25*x_27 + x_26*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_5*x_28 + x_10*x_28 + x_15*x_28 + x_18*x_28 + x_19*x_28 + x_24*x_28 + x_25*x_28 + x_1*x_29 + x_2*x_29 + x_5*x_29 + x_8*x_29 + \n"
                       "        x_9*x_29 + x_15*x_29 + x_19*x_29 + x_22*x_29 + x_23*x_29 + x_26*x_29 + x_2*x_30 + x_4*x_30 + x_5*x_30 + x_7*x_30 + x_9*x_30 + x_11*x_30 + x_12*x_30 + x_13*x_30 + x_15*x_30 + x_18*x_30 + x_19*x_30 \n"
                       "        + x_21*x_30 + x_24*x_30 + x_26*x_30 + x_27*x_30 + x_28*x_30 + x_1*x_31 + x_2*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_7*x_31 + x_8*x_31 + x_11*x_31 + x_18*x_31 + x_20*x_31 + x_22*x_31 + x_23*x_31\n"
                       "        + x_24*x_31 + x_25*x_31 + x_26*x_31 + x_29*x_31 + x_30*x_31 + x_1*x_32 + x_4*x_32 + x_7*x_32 + x_9*x_32 + x_10*x_32 + x_11*x_32 + x_13*x_32 + x_14*x_32 + x_15*x_32 + x_16*x_32 + x_17*x_32 + \n"
                       "        x_18*x_32 + x_20*x_32 + x_21*x_32 + x_22*x_32 + x_25*x_32 + x_28*x_32 + x_30*x_32 + x_1*x_33 + x_3*x_33 + x_4*x_33 + x_7*x_33 + x_8*x_33 + x_10*x_33 + x_12*x_33 + x_13*x_33 + x_14*x_33 + x_15*x_33\n"
                       "        + x_16*x_33 + x_17*x_33 + x_18*x_33 + x_19*x_33 + x_22*x_33 + x_23*x_33 + x_26*x_33 + x_27*x_33 + x_29*x_33 + x_2*x_34 + x_9*x_34 + x_11*x_34 + x_15*x_34 + x_17*x_34 + x_18*x_34 + x_20*x_34 + \n"
                       "        x_21*x_34 + x_22*x_34 + x_26*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_1*x_35 + x_2*x_35 + x_8*x_35 + x_9*x_35 + x_10*x_35 + x_12*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_20*x_35 + \n"
                       "        x_22*x_35 + x_24*x_35 + x_25*x_35 + x_27*x_35 + x_30*x_35 + x_32*x_35 + x_1*x_36 + x_2*x_36 + x_3*x_36 + x_4*x_36 + x_6*x_36 + x_7*x_36 + x_8*x_36 + x_9*x_36 + x_11*x_36 + x_13*x_36 + x_14*x_36 + \n"
                       "        x_24*x_36 + x_26*x_36 + x_31*x_36 + x_34*x_36 + x_35*x_36 + x_1*x_37 + x_8*x_37 + x_10*x_37 + x_13*x_37 + x_14*x_37 + x_15*x_37 + x_18*x_37 + x_20*x_37 + x_21*x_37 + x_24*x_37 + x_30*x_37 + \n"
                       "        x_32*x_37 + x_33*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_5*x_38 + x_7*x_38 + x_8*x_38 + x_10*x_38 + x_11*x_38 + x_12*x_38 + x_21*x_38 + x_22*x_38 + x_23*x_38 + x_24*x_38 + x_25*x_38 + x_30*x_38 \n"
                       "        + x_32*x_38 + x_35*x_38 + x_37*x_38 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_6*x_39 + x_8*x_39 + x_9*x_39 + x_11*x_39 + x_13*x_39 + x_17*x_39 + x_18*x_39 + x_20*x_39 + x_21*x_39 + x_22*x_39 + \n"
                       "        x_24*x_39 + x_25*x_39 + x_28*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_33*x_39 + x_37*x_39 + x_38*x_39 + x_1*x_40 + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_8*x_40 + x_9*x_40 + x_10*x_40 + x_13*x_40 \n"
                       "        + x_17*x_40 + x_20*x_40 + x_24*x_40 + x_27*x_40 + x_28*x_40 + x_29*x_40 + x_32*x_40 + x_34*x_40 + x_36*x_40 + x_37*x_40 + x_38*x_40 + x_39*x_40 + x_1*x_41 + x_3*x_41 + x_4*x_41 + x_5*x_41 + \n"
                       "        x_9*x_41 + x_11*x_41 + x_12*x_41 + x_13*x_41 + x_14*x_41 + x_17*x_41 + x_18*x_41 + x_19*x_41 + x_21*x_41 + x_22*x_41 + x_25*x_41 + x_26*x_41 + x_28*x_41 + x_30*x_41 + x_31*x_41 + x_33*x_41 + \n"
                       "        x_35*x_41 + x_38*x_41 + x_40*x_41 + x_3*x_42 + x_6*x_42 + x_10*x_42 + x_14*x_42 + x_22*x_42 + x_23*x_42 + x_27*x_42 + x_28*x_42 + x_32*x_42 + x_34*x_42 + x_37*x_42 + x_39*x_42 + x_40*x_42 + \n"
                       "        x_41*x_42 + x_1*x_43 + x_3*x_43 + x_9*x_43 + x_10*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_17*x_43 + x_20*x_43 + x_22*x_43 + x_23*x_43 + x_25*x_43 + x_26*x_43 + x_27*x_43 + x_28*x_43 + \n"
                       "        x_29*x_43 + x_30*x_43 + x_32*x_43 + x_33*x_43 + x_36*x_43 + x_37*x_43 + x_1*x_44 + x_2*x_44 + x_3*x_44 + x_6*x_44 + x_7*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_16*x_44 + x_19*x_44\n"
                       "        + x_20*x_44 + x_21*x_44 + x_23*x_44 + x_24*x_44 + x_25*x_44 + x_27*x_44 + x_28*x_44 + x_33*x_44 + x_34*x_44 + x_35*x_44 + x_36*x_44 + x_38*x_44 + x_40*x_44 + x_42*x_44 + x_43*x_44 + x_1*x_45 + \n"
                       "        x_2*x_45 + x_6*x_45 + x_7*x_45 + x_8*x_45 + x_9*x_45 + x_10*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_15*x_45 + x_17*x_45 + x_18*x_45 + x_20*x_45 + x_24*x_45 + x_25*x_45 + x_26*x_45 + x_28*x_45\n"
                       "        + x_30*x_45 + x_34*x_45 + x_35*x_45 + x_36*x_45 + x_38*x_45 + x_39*x_45 + x_41*x_45 + x_2*x_46 + x_4*x_46 + x_6*x_46 + x_9*x_46 + x_10*x_46 + x_11*x_46 + x_12*x_46 + x_13*x_46 + x_15*x_46 + \n"
                       "        x_19*x_46 + x_20*x_46 + x_21*x_46 + x_22*x_46 + x_24*x_46 + x_29*x_46 + x_30*x_46 + x_33*x_46 + x_34*x_46 + x_35*x_46 + x_37*x_46 + x_39*x_46 + x_41*x_46 + x_42*x_46 + x_44*x_46 + x_4*x_47 + \n"
                       "        x_5*x_47 + x_9*x_47 + x_13*x_47 + x_15*x_47 + x_16*x_47 + x_22*x_47 + x_24*x_47 + x_26*x_47 + x_28*x_47 + x_30*x_47 + x_31*x_47 + x_32*x_47 + x_35*x_47 + x_36*x_47 + x_41*x_47 + x_44*x_47 + \n"
                       "        x_45*x_47 + x_46*x_47 + x_2*x_48 + x_3*x_48 + x_6*x_48 + x_8*x_48 + x_10*x_48 + x_13*x_48 + x_14*x_48 + x_15*x_48 + x_17*x_48 + x_18*x_48 + x_20*x_48 + x_22*x_48 + x_23*x_48 + x_24*x_48 + \n"
                       "        x_26*x_48 + x_28*x_48 + x_30*x_48 + x_32*x_48 + x_33*x_48 + x_36*x_48 + x_38*x_48 + x_41*x_48 + x_42*x_48 + x_44*x_48 + x_45*x_48 + x_46*x_48 + x_47*x_48 + x_5*x_49 + x_6*x_49 + x_7*x_49 + \n"
                       "        x_8*x_49 + x_9*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49 + x_18*x_49 + x_22*x_49 + x_23*x_49 + x_24*x_49 + x_27*x_49 + x_28*x_49 + x_29*x_49 + x_30*x_49 + x_34*x_49 + x_38*x_49 + \n"
                       "        x_40*x_49 + x_43*x_49 + x_44*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_3 + x_5 + x_8 + x_9 + x_10 + x_11 + x_16 + x_21 + x_24 + x_26 + x_28 + x_29 + x_31 + x_35 + x_36 + x_42 + x_47 + \n"
                       "        x_48,\n"
                       "    x_1*x_2 + x_2*x_3 + x_1*x_4 + x_2*x_4 + x_1*x_5 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_4*x_6 + x_5*x_6 + x_1*x_7 + x_3*x_7 + x_4*x_7 + x_6*x_7 + x_1*x_8 + x_6*x_8 + x_2*x_9 + x_3*x_9 + x_6*x_9 + x_1*x_10 +\n"
                       "        x_2*x_10 + x_4*x_10 + x_5*x_10 + x_6*x_10 + x_8*x_10 + x_9*x_10 + x_1*x_11 + x_2*x_11 + x_4*x_11 + x_5*x_11 + x_7*x_11 + x_9*x_11 + x_3*x_12 + x_4*x_12 + x_5*x_12 + x_6*x_12 + x_9*x_12 + x_10*x_12\n"
                       "        + x_11*x_12 + x_1*x_13 + x_2*x_13 + x_3*x_13 + x_4*x_13 + x_12*x_13 + x_1*x_14 + x_2*x_14 + x_4*x_14 + x_6*x_14 + x_7*x_14 + x_10*x_14 + x_12*x_14 + x_13*x_14 + x_5*x_15 + x_6*x_15 + x_13*x_15 + \n"
                       "        x_1*x_16 + x_3*x_16 + x_4*x_16 + x_6*x_16 + x_7*x_16 + x_9*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_6*x_17 + x_8*x_17 + x_10*x_17 + x_11*x_17 + \n"
                       "        x_15*x_17 + x_16*x_17 + x_1*x_18 + x_2*x_18 + x_3*x_18 + x_4*x_18 + x_5*x_18 + x_9*x_18 + x_11*x_18 + x_12*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + x_3*x_19 + x_5*x_19 + x_8*x_19 + x_9*x_19 + \n"
                       "        x_10*x_19 + x_15*x_19 + x_2*x_20 + x_4*x_20 + x_7*x_20 + x_8*x_20 + x_10*x_20 + x_13*x_20 + x_15*x_20 + x_17*x_20 + x_19*x_20 + x_1*x_21 + x_4*x_21 + x_6*x_21 + x_7*x_21 + x_11*x_21 + x_12*x_21 + \n"
                       "        x_15*x_21 + x_16*x_21 + x_19*x_21 + x_4*x_22 + x_5*x_22 + x_6*x_22 + x_7*x_22 + x_8*x_22 + x_11*x_22 + x_14*x_22 + x_15*x_22 + x_17*x_22 + x_18*x_22 + x_19*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 \n"
                       "        + x_8*x_23 + x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_17*x_23 + x_18*x_23 + x_19*x_23 + x_22*x_23 + x_4*x_24 + x_7*x_24 + x_8*x_24 + x_9*x_24 + x_10*x_24 + x_11*x_24 + x_15*x_24 + \n"
                       "        x_16*x_24 + x_17*x_24 + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_21*x_24 + x_23*x_24 + x_1*x_25 + x_3*x_25 + x_4*x_25 + x_6*x_25 + x_8*x_25 + x_9*x_25 + x_10*x_25 + x_13*x_25 + x_15*x_25 + x_18*x_25 \n"
                       "        + x_21*x_25 + x_1*x_26 + x_3*x_26 + x_5*x_26 + x_9*x_26 + x_10*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_16*x_26 + x_18*x_26 + x_19*x_26 + x_20*x_26 + x_22*x_26 + x_25*x_26 + x_1*x_27 + \n"
                       "        x_2*x_27 + x_5*x_27 + x_10*x_27 + x_12*x_27 + x_13*x_27 + x_14*x_27 + x_17*x_27 + x_20*x_27 + x_21*x_27 + x_23*x_27 + x_24*x_27 + x_25*x_27 + x_26*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_5*x_28 \n"
                       "        + x_6*x_28 + x_8*x_28 + x_9*x_28 + x_10*x_28 + x_12*x_28 + x_15*x_28 + x_17*x_28 + x_18*x_28 + x_19*x_28 + x_24*x_28 + x_25*x_28 + x_1*x_29 + x_2*x_29 + x_5*x_29 + x_7*x_29 + x_8*x_29 + x_10*x_29 \n"
                       "        + x_12*x_29 + x_13*x_29 + x_15*x_29 + x_16*x_29 + x_17*x_29 + x_24*x_29 + x_26*x_29 + x_4*x_30 + x_7*x_30 + x_12*x_30 + x_13*x_30 + x_14*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + x_19*x_30 + \n"
                       "        x_21*x_30 + x_22*x_30 + x_27*x_30 + x_28*x_30 + x_29*x_30 + x_1*x_31 + x_2*x_31 + x_4*x_31 + x_5*x_31 + x_6*x_31 + x_8*x_31 + x_12*x_31 + x_14*x_31 + x_16*x_31 + x_17*x_31 + x_19*x_31 + x_20*x_31 \n"
                       "        + x_21*x_31 + x_22*x_31 + x_24*x_31 + x_28*x_31 + x_2*x_32 + x_6*x_32 + x_10*x_32 + x_11*x_32 + x_13*x_32 + x_16*x_32 + x_17*x_32 + x_18*x_32 + x_19*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32 + \n"
                       "        x_25*x_32 + x_26*x_32 + x_27*x_32 + x_28*x_32 + x_29*x_32 + x_30*x_32 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_6*x_33 + x_10*x_33 + x_13*x_33 + x_17*x_33 + x_18*x_33 + x_19*x_33 + x_20*x_33 + \n"
                       "        x_21*x_33 + x_22*x_33 + x_23*x_33 + x_24*x_33 + x_25*x_33 + x_27*x_33 + x_28*x_33 + x_2*x_34 + x_5*x_34 + x_7*x_34 + x_10*x_34 + x_11*x_34 + x_20*x_34 + x_21*x_34 + x_23*x_34 + x_24*x_34 + \n"
                       "        x_26*x_34 + x_27*x_34 + x_30*x_34 + x_31*x_34 + x_32*x_34 + x_3*x_35 + x_4*x_35 + x_5*x_35 + x_7*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_16*x_35 + x_17*x_35 + x_18*x_35 + x_21*x_35 + \n"
                       "        x_23*x_35 + x_24*x_35 + x_26*x_35 + x_29*x_35 + x_31*x_35 + x_33*x_35 + x_5*x_36 + x_7*x_36 + x_9*x_36 + x_12*x_36 + x_13*x_36 + x_16*x_36 + x_17*x_36 + x_18*x_36 + x_19*x_36 + x_26*x_36 + \n"
                       "        x_30*x_36 + x_32*x_36 + x_33*x_36 + x_1*x_37 + x_2*x_37 + x_3*x_37 + x_4*x_37 + x_7*x_37 + x_8*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 + x_13*x_37 + x_15*x_37 + x_16*x_37 + x_22*x_37 + x_23*x_37 \n"
                       "        + x_24*x_37 + x_25*x_37 + x_30*x_37 + x_31*x_37 + x_33*x_37 + x_35*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + x_11*x_38 + x_12*x_38 + x_13*x_38 + x_14*x_38\n"
                       "        + x_23*x_38 + x_25*x_38 + x_26*x_38 + x_28*x_38 + x_30*x_38 + x_33*x_38 + x_34*x_38 + x_36*x_38 + x_37*x_38 + x_2*x_39 + x_3*x_39 + x_4*x_39 + x_5*x_39 + x_8*x_39 + x_11*x_39 + x_12*x_39 + \n"
                       "        x_13*x_39 + x_16*x_39 + x_18*x_39 + x_21*x_39 + x_22*x_39 + x_26*x_39 + x_29*x_39 + x_33*x_39 + x_35*x_39 + x_38*x_39 + x_4*x_40 + x_7*x_40 + x_8*x_40 + x_10*x_40 + x_19*x_40 + x_20*x_40 + \n"
                       "        x_22*x_40 + x_25*x_40 + x_28*x_40 + x_30*x_40 + x_31*x_40 + x_34*x_40 + x_35*x_40 + x_37*x_40 + x_39*x_40 + x_5*x_41 + x_8*x_41 + x_9*x_41 + x_13*x_41 + x_14*x_41 + x_16*x_41 + x_19*x_41 + \n"
                       "        x_20*x_41 + x_21*x_41 + x_23*x_41 + x_25*x_41 + x_29*x_41 + x_32*x_41 + x_36*x_41 + x_37*x_41 + x_2*x_42 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_13*x_42 + x_16*x_42 + x_19*x_42 + x_20*x_42 + \n"
                       "        x_23*x_42 + x_25*x_42 + x_26*x_42 + x_28*x_42 + x_32*x_42 + x_33*x_42 + x_34*x_42 + x_37*x_42 + x_38*x_42 + x_40*x_42 + x_41*x_42 + x_1*x_43 + x_2*x_43 + x_3*x_43 + x_4*x_43 + x_5*x_43 + x_7*x_43 \n"
                       "        + x_11*x_43 + x_13*x_43 + x_14*x_43 + x_16*x_43 + x_20*x_43 + x_21*x_43 + x_24*x_43 + x_25*x_43 + x_26*x_43 + x_27*x_43 + x_28*x_43 + x_30*x_43 + x_32*x_43 + x_34*x_43 + x_39*x_43 + x_40*x_43 + \n"
                       "        x_41*x_43 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_5*x_44 + x_6*x_44 + x_8*x_44 + x_9*x_44 + x_10*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_20*x_44 + x_21*x_44 + x_25*x_44 + x_30*x_44 + x_31*x_44 +\n"
                       "        x_32*x_44 + x_33*x_44 + x_36*x_44 + x_37*x_44 + x_38*x_44 + x_43*x_44 + x_3*x_45 + x_4*x_45 + x_5*x_45 + x_7*x_45 + x_8*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_14*x_45 + x_20*x_45 + x_22*x_45\n"
                       "        + x_27*x_45 + x_28*x_45 + x_31*x_45 + x_32*x_45 + x_33*x_45 + x_34*x_45 + x_35*x_45 + x_38*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_42*x_45 + x_43*x_45 + x_44*x_45 + x_7*x_46 + x_10*x_46 + \n"
                       "        x_12*x_46 + x_13*x_46 + x_15*x_46 + x_16*x_46 + x_18*x_46 + x_22*x_46 + x_23*x_46 + x_25*x_46 + x_27*x_46 + x_28*x_46 + x_29*x_46 + x_30*x_46 + x_32*x_46 + x_33*x_46 + x_35*x_46 + x_38*x_46 + \n"
                       "        x_40*x_46 + x_42*x_46 + x_44*x_46 + x_45*x_46 + x_1*x_47 + x_3*x_47 + x_4*x_47 + x_5*x_47 + x_9*x_47 + x_10*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_19*x_47 + x_22*x_47 + x_23*x_47 + x_25*x_47\n"
                       "        + x_26*x_47 + x_28*x_47 + x_29*x_47 + x_37*x_47 + x_38*x_47 + x_40*x_47 + x_41*x_47 + x_46*x_47 + x_1*x_48 + x_5*x_48 + x_6*x_48 + x_7*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + \n"
                       "        x_14*x_48 + x_16*x_48 + x_18*x_48 + x_19*x_48 + x_20*x_48 + x_21*x_48 + x_22*x_48 + x_24*x_48 + x_26*x_48 + x_27*x_48 + x_29*x_48 + x_38*x_48 + x_41*x_48 + x_43*x_48 + x_44*x_48 + x_45*x_48 + \n"
                       "        x_46*x_48 + x_2*x_49 + x_4*x_49 + x_6*x_49 + x_7*x_49 + x_9*x_49 + x_11*x_49 + x_15*x_49 + x_17*x_49 + x_20*x_49 + x_21*x_49 + x_23*x_49 + x_25*x_49 + x_26*x_49 + x_27*x_49 + x_30*x_49 + x_31*x_49\n"
                       "        + x_33*x_49 + x_34*x_49 + x_38*x_49 + x_42*x_49 + x_47*x_49 + x_2 + x_3 + x_5 + x_7 + x_8 + x_9 + x_10 + x_11 + x_13 + x_14 + x_18 + x_19 + x_23 + x_24 + x_25 + x_26 + x_32 + x_34 + x_35 + x_39 + \n"
                       "        x_40 + x_41 + x_44 + x_45 + x_49,\n"
                       "    x_1*x_2 + x_3*x_4 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + x_1*x_7 + x_4*x_7 + x_1*x_8 + x_3*x_8 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_1*x_9 + x_2*x_9 + x_4*x_9 + x_1*x_10 + x_7*x_10 + x_9*x_10 + \n"
                       "        x_2*x_11 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_7*x_11 + x_3*x_12 + x_7*x_12 + x_9*x_12 + x_10*x_12 + x_2*x_13 + x_3*x_13 + x_5*x_13 + x_6*x_13 + x_7*x_13 + x_8*x_13 + x_11*x_13 + x_1*x_14 + \n"
                       "        x_2*x_14 + x_3*x_14 + x_4*x_14 + x_6*x_14 + x_9*x_14 + x_10*x_14 + x_12*x_14 + x_13*x_14 + x_2*x_15 + x_6*x_15 + x_7*x_15 + x_9*x_15 + x_11*x_15 + x_13*x_15 + x_14*x_15 + x_1*x_16 + x_3*x_16 + \n"
                       "        x_4*x_16 + x_5*x_16 + x_7*x_16 + x_8*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_15*x_16 + x_2*x_17 + x_6*x_17 + x_8*x_17 + x_9*x_17 + x_12*x_17 + x_13*x_17 + x_15*x_17 + x_2*x_18 + x_3*x_18 + \n"
                       "        x_4*x_18 + x_6*x_18 + x_9*x_18 + x_10*x_18 + x_13*x_18 + x_14*x_18 + x_16*x_18 + x_1*x_19 + x_2*x_19 + x_3*x_19 + x_4*x_19 + x_5*x_19 + x_8*x_19 + x_9*x_19 + x_12*x_19 + x_13*x_19 + x_15*x_19 + \n"
                       "        x_16*x_19 + x_1*x_20 + x_2*x_20 + x_6*x_20 + x_10*x_20 + x_11*x_20 + x_13*x_20 + x_15*x_20 + x_17*x_20 + x_18*x_20 + x_19*x_20 + x_4*x_21 + x_6*x_21 + x_8*x_21 + x_9*x_21 + x_10*x_21 + x_14*x_21 +\n"
                       "        x_17*x_21 + x_20*x_21 + x_3*x_22 + x_5*x_22 + x_7*x_22 + x_8*x_22 + x_9*x_22 + x_10*x_22 + x_11*x_22 + x_13*x_22 + x_15*x_22 + x_16*x_22 + x_18*x_22 + x_20*x_22 + x_21*x_22 + x_6*x_23 + x_8*x_23 +\n"
                       "        x_10*x_23 + x_11*x_23 + x_12*x_23 + x_13*x_23 + x_15*x_23 + x_19*x_23 + x_21*x_23 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_6*x_24 + x_7*x_24 + x_8*x_24 + x_12*x_24 + x_13*x_24 + x_19*x_24 + x_20*x_24 \n"
                       "        + x_21*x_24 + x_1*x_25 + x_3*x_25 + x_4*x_25 + x_5*x_25 + x_6*x_25 + x_10*x_25 + x_11*x_25 + x_18*x_25 + x_19*x_25 + x_20*x_25 + x_22*x_25 + x_23*x_25 + x_1*x_26 + x_6*x_26 + x_7*x_26 + x_9*x_26 +\n"
                       "        x_14*x_26 + x_16*x_26 + x_18*x_26 + x_20*x_26 + x_21*x_26 + x_23*x_26 + x_25*x_26 + x_1*x_27 + x_3*x_27 + x_6*x_27 + x_9*x_27 + x_11*x_27 + x_12*x_27 + x_13*x_27 + x_14*x_27 + x_15*x_27 + \n"
                       "        x_16*x_27 + x_18*x_27 + x_19*x_27 + x_22*x_27 + x_2*x_28 + x_5*x_28 + x_7*x_28 + x_8*x_28 + x_10*x_28 + x_12*x_28 + x_15*x_28 + x_18*x_28 + x_19*x_28 + x_20*x_28 + x_21*x_28 + x_23*x_28 + \n"
                       "        x_24*x_28 + x_27*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_4*x_29 + x_5*x_29 + x_8*x_29 + x_9*x_29 + x_10*x_29 + x_14*x_29 + x_15*x_29 + x_16*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 + x_20*x_29 +\n"
                       "        x_24*x_29 + x_25*x_29 + x_4*x_30 + x_5*x_30 + x_8*x_30 + x_13*x_30 + x_14*x_30 + x_16*x_30 + x_18*x_30 + x_19*x_30 + x_22*x_30 + x_25*x_30 + x_26*x_30 + x_29*x_30 + x_5*x_31 + x_6*x_31 + x_8*x_31 \n"
                       "        + x_9*x_31 + x_10*x_31 + x_13*x_31 + x_16*x_31 + x_20*x_31 + x_21*x_31 + x_22*x_31 + x_24*x_31 + x_26*x_31 + x_27*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32 + x_3*x_32 + \n"
                       "        x_4*x_32 + x_7*x_32 + x_8*x_32 + x_10*x_32 + x_12*x_32 + x_14*x_32 + x_16*x_32 + x_22*x_32 + x_25*x_32 + x_26*x_32 + x_29*x_32 + x_31*x_32 + x_1*x_33 + x_5*x_33 + x_6*x_33 + x_8*x_33 + x_9*x_33 + \n"
                       "        x_10*x_33 + x_12*x_33 + x_13*x_33 + x_16*x_33 + x_18*x_33 + x_21*x_33 + x_23*x_33 + x_24*x_33 + x_26*x_33 + x_27*x_33 + x_28*x_33 + x_32*x_33 + x_1*x_34 + x_4*x_34 + x_7*x_34 + x_8*x_34 + \n"
                       "        x_11*x_34 + x_12*x_34 + x_14*x_34 + x_17*x_34 + x_18*x_34 + x_21*x_34 + x_22*x_34 + x_23*x_34 + x_25*x_34 + x_27*x_34 + x_28*x_34 + x_29*x_34 + x_31*x_34 + x_32*x_34 + x_33*x_34 + x_2*x_35 + \n"
                       "        x_3*x_35 + x_5*x_35 + x_7*x_35 + x_9*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_20*x_35 + x_21*x_35 + x_23*x_35 + x_29*x_35 + x_30*x_35 + x_31*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_3*x_36 \n"
                       "        + x_4*x_36 + x_6*x_36 + x_7*x_36 + x_9*x_36 + x_11*x_36 + x_12*x_36 + x_13*x_36 + x_14*x_36 + x_15*x_36 + x_16*x_36 + x_20*x_36 + x_21*x_36 + x_24*x_36 + x_25*x_36 + x_26*x_36 + x_28*x_36 + \n"
                       "        x_29*x_36 + x_30*x_36 + x_35*x_36 + x_6*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_15*x_37 + x_18*x_37 + x_20*x_37 + x_22*x_37 + x_24*x_37 + x_26*x_37 + x_28*x_37 + x_29*x_37 + x_30*x_37 + \n"
                       "        x_36*x_37 + x_1*x_38 + x_2*x_38 + x_3*x_38 + x_4*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_9*x_38 + x_10*x_38 + x_11*x_38 + x_13*x_38 + x_14*x_38 + x_16*x_38 + x_18*x_38 + x_19*x_38 + x_21*x_38 + \n"
                       "        x_24*x_38 + x_26*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_32*x_38 + x_33*x_38 + x_35*x_38 + x_2*x_39 + x_5*x_39 + x_6*x_39 + x_9*x_39 + x_11*x_39 + x_14*x_39 + x_18*x_39 + \n"
                       "        x_20*x_39 + x_25*x_39 + x_29*x_39 + x_30*x_39 + x_31*x_39 + x_32*x_39 + x_33*x_39 + x_34*x_39 + x_37*x_39 + x_1*x_40 + x_4*x_40 + x_5*x_40 + x_6*x_40 + x_7*x_40 + x_14*x_40 + x_18*x_40 + x_20*x_40\n"
                       "        + x_22*x_40 + x_25*x_40 + x_26*x_40 + x_28*x_40 + x_30*x_40 + x_32*x_40 + x_33*x_40 + x_34*x_40 + x_35*x_40 + x_38*x_40 + x_39*x_40 + x_1*x_41 + x_3*x_41 + x_4*x_41 + x_6*x_41 + x_11*x_41 + \n"
                       "        x_13*x_41 + x_15*x_41 + x_17*x_41 + x_19*x_41 + x_26*x_41 + x_27*x_41 + x_28*x_41 + x_34*x_41 + x_35*x_41 + x_39*x_41 + x_40*x_41 + x_2*x_42 + x_3*x_42 + x_5*x_42 + x_6*x_42 + x_9*x_42 + x_10*x_42\n"
                       "        + x_13*x_42 + x_15*x_42 + x_16*x_42 + x_17*x_42 + x_18*x_42 + x_19*x_42 + x_23*x_42 + x_24*x_42 + x_26*x_42 + x_29*x_42 + x_31*x_42 + x_34*x_42 + x_35*x_42 + x_36*x_42 + x_37*x_42 + x_3*x_43 + \n"
                       "        x_5*x_43 + x_6*x_43 + x_8*x_43 + x_9*x_43 + x_13*x_43 + x_16*x_43 + x_18*x_43 + x_21*x_43 + x_23*x_43 + x_24*x_43 + x_26*x_43 + x_28*x_43 + x_29*x_43 + x_36*x_43 + x_38*x_43 + x_39*x_43 + \n"
                       "        x_40*x_43 + x_42*x_43 + x_3*x_44 + x_5*x_44 + x_6*x_44 + x_7*x_44 + x_8*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_13*x_44 + x_14*x_44 + x_15*x_44 + x_21*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44\n"
                       "        + x_31*x_44 + x_34*x_44 + x_37*x_44 + x_38*x_44 + x_40*x_44 + x_41*x_44 + x_42*x_44 + x_43*x_44 + x_3*x_45 + x_4*x_45 + x_6*x_45 + x_9*x_45 + x_11*x_45 + x_12*x_45 + x_13*x_45 + x_15*x_45 + \n"
                       "        x_19*x_45 + x_21*x_45 + x_25*x_45 + x_26*x_45 + x_32*x_45 + x_34*x_45 + x_38*x_45 + x_40*x_45 + x_42*x_45 + x_44*x_45 + x_1*x_46 + x_3*x_46 + x_5*x_46 + x_6*x_46 + x_10*x_46 + x_11*x_46 + \n"
                       "        x_13*x_46 + x_14*x_46 + x_15*x_46 + x_20*x_46 + x_22*x_46 + x_23*x_46 + x_24*x_46 + x_26*x_46 + x_28*x_46 + x_29*x_46 + x_33*x_46 + x_35*x_46 + x_36*x_46 + x_37*x_46 + x_38*x_46 + x_40*x_46 + \n"
                       "        x_41*x_46 + x_43*x_46 + x_45*x_46 + x_1*x_47 + x_3*x_47 + x_5*x_47 + x_6*x_47 + x_8*x_47 + x_12*x_47 + x_13*x_47 + x_14*x_47 + x_16*x_47 + x_18*x_47 + x_21*x_47 + x_22*x_47 + x_23*x_47 + x_25*x_47\n"
                       "        + x_28*x_47 + x_30*x_47 + x_32*x_47 + x_33*x_47 + x_34*x_47 + x_35*x_47 + x_41*x_47 + x_4*x_48 + x_7*x_48 + x_9*x_48 + x_13*x_48 + x_14*x_48 + x_15*x_48 + x_22*x_48 + x_23*x_48 + x_25*x_48 + \n"
                       "        x_26*x_48 + x_28*x_48 + x_29*x_48 + x_30*x_48 + x_32*x_48 + x_33*x_48 + x_35*x_48 + x_41*x_48 + x_42*x_48 + x_44*x_48 + x_45*x_48 + x_1*x_49 + x_4*x_49 + x_6*x_49 + x_8*x_49 + x_11*x_49 + \n"
                       "        x_12*x_49 + x_14*x_49 + x_15*x_49 + x_17*x_49 + x_19*x_49 + x_20*x_49 + x_22*x_49 + x_23*x_49 + x_24*x_49 + x_29*x_49 + x_31*x_49 + x_34*x_49 + x_36*x_49 + x_38*x_49 + x_40*x_49 + x_42*x_49 + \n"
                       "        x_44*x_49 + x_46*x_49 + x_47*x_49 + x_48*x_49 + x_2 + x_6 + x_7 + x_9 + x_10 + x_12 + x_13 + x_18 + x_20 + x_22 + x_28 + x_30 + x_34 + x_35 + x_36 + x_40 + x_41 + x_43 + x_47,\n"
                       "    x_1*x_2 + x_1*x_4 + x_2*x_4 + x_2*x_5 + x_3*x_5 + x_2*x_6 + x_3*x_6 + x_3*x_7 + x_4*x_7 + x_2*x_8 + x_3*x_9 + x_4*x_9 + x_8*x_9 + x_1*x_10 + x_2*x_10 + x_4*x_10 + x_6*x_10 + x_8*x_10 + x_9*x_10 + \n"
                       "        x_1*x_11 + x_3*x_11 + x_4*x_11 + x_5*x_11 + x_6*x_11 + x_8*x_11 + x_9*x_11 + x_1*x_12 + x_3*x_12 + x_6*x_12 + x_7*x_12 + x_8*x_12 + x_10*x_12 + x_1*x_13 + x_2*x_13 + x_8*x_13 + x_9*x_13 + \n"
                       "        x_10*x_13 + x_11*x_13 + x_1*x_14 + x_4*x_14 + x_5*x_14 + x_6*x_14 + x_10*x_14 + x_12*x_14 + x_1*x_15 + x_3*x_15 + x_6*x_15 + x_7*x_15 + x_9*x_15 + x_10*x_15 + x_11*x_15 + x_12*x_15 + x_3*x_16 + \n"
                       "        x_4*x_16 + x_6*x_16 + x_8*x_16 + x_10*x_16 + x_11*x_16 + x_13*x_16 + x_14*x_16 + x_15*x_16 + x_1*x_17 + x_2*x_17 + x_6*x_17 + x_9*x_17 + x_13*x_17 + x_14*x_17 + x_1*x_18 + x_4*x_18 + x_6*x_18 + \n"
                       "        x_8*x_18 + x_14*x_18 + x_15*x_18 + x_16*x_18 + x_17*x_18 + x_1*x_19 + x_6*x_19 + x_7*x_19 + x_11*x_19 + x_14*x_19 + x_15*x_19 + x_1*x_20 + x_2*x_20 + x_6*x_20 + x_9*x_20 + x_10*x_20 + x_11*x_20 + \n"
                       "        x_12*x_20 + x_1*x_21 + x_3*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_12*x_21 + x_15*x_21 + x_17*x_21 + x_19*x_21 + x_20*x_21 + x_3*x_22 + x_4*x_22 + x_6*x_22 + x_9*x_22 + x_11*x_22 + \n"
                       "        x_16*x_22 + x_20*x_22 + x_21*x_22 + x_1*x_23 + x_2*x_23 + x_3*x_23 + x_4*x_23 + x_6*x_23 + x_7*x_23 + x_8*x_23 + x_12*x_23 + x_14*x_23 + x_16*x_23 + x_18*x_23 + x_1*x_24 + x_2*x_24 + x_3*x_24 + \n"
                       "        x_4*x_24 + x_5*x_24 + x_11*x_24 + x_12*x_24 + x_14*x_24 + x_15*x_24 + x_17*x_24 + x_19*x_24 + x_20*x_24 + x_1*x_25 + x_7*x_25 + x_12*x_25 + x_13*x_25 + x_18*x_25 + x_19*x_25 + x_20*x_25 + \n"
                       "        x_23*x_25 + x_1*x_26 + x_3*x_26 + x_4*x_26 + x_6*x_26 + x_7*x_26 + x_8*x_26 + x_10*x_26 + x_11*x_26 + x_14*x_26 + x_15*x_26 + x_17*x_26 + x_24*x_26 + x_25*x_26 + x_5*x_27 + x_8*x_27 + x_9*x_27 + \n"
                       "        x_11*x_27 + x_13*x_27 + x_15*x_27 + x_17*x_27 + x_19*x_27 + x_22*x_27 + x_23*x_27 + x_25*x_27 + x_4*x_28 + x_5*x_28 + x_6*x_28 + x_9*x_28 + x_12*x_28 + x_13*x_28 + x_16*x_28 + x_17*x_28 + \n"
                       "        x_18*x_28 + x_21*x_28 + x_22*x_28 + x_26*x_28 + x_1*x_29 + x_2*x_29 + x_3*x_29 + x_6*x_29 + x_8*x_29 + x_13*x_29 + x_15*x_29 + x_17*x_29 + x_18*x_29 + x_21*x_29 + x_23*x_29 + x_25*x_29 + x_26*x_29\n"
                       "        + x_27*x_29 + x_4*x_30 + x_5*x_30 + x_6*x_30 + x_7*x_30 + x_9*x_30 + x_12*x_30 + x_16*x_30 + x_17*x_30 + x_19*x_30 + x_21*x_30 + x_26*x_30 + x_27*x_30 + x_28*x_30 + x_1*x_31 + x_3*x_31 + x_4*x_31 \n"
                       "        + x_6*x_31 + x_8*x_31 + x_9*x_31 + x_12*x_31 + x_13*x_31 + x_14*x_31 + x_17*x_31 + x_18*x_31 + x_19*x_31 + x_21*x_31 + x_22*x_31 + x_25*x_31 + x_27*x_31 + x_30*x_31 + x_2*x_32 + x_3*x_32 + \n"
                       "        x_4*x_32 + x_10*x_32 + x_11*x_32 + x_12*x_32 + x_13*x_32 + x_15*x_32 + x_16*x_32 + x_18*x_32 + x_19*x_32 + x_20*x_32 + x_21*x_32 + x_23*x_32 + x_24*x_32 + x_25*x_32 + x_27*x_32 + x_30*x_32 + \n"
                       "        x_31*x_32 + x_1*x_33 + x_2*x_33 + x_3*x_33 + x_4*x_33 + x_8*x_33 + x_12*x_33 + x_14*x_33 + x_16*x_33 + x_19*x_33 + x_20*x_33 + x_21*x_33 + x_22*x_33 + x_25*x_33 + x_26*x_33 + x_27*x_33 + x_29*x_33\n"
                       "        + x_30*x_33 + x_1*x_34 + x_2*x_34 + x_5*x_34 + x_8*x_34 + x_13*x_34 + x_14*x_34 + x_15*x_34 + x_16*x_34 + x_17*x_34 + x_20*x_34 + x_22*x_34 + x_23*x_34 + x_25*x_34 + x_28*x_34 + x_31*x_34 + \n"
                       "        x_2*x_35 + x_6*x_35 + x_8*x_35 + x_9*x_35 + x_11*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_15*x_35 + x_17*x_35 + x_18*x_35 + x_19*x_35 + x_20*x_35 + x_23*x_35 + x_24*x_35 + x_26*x_35 + \n"
                       "        x_27*x_35 + x_29*x_35 + x_30*x_35 + x_31*x_35 + x_32*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_6*x_36 + x_8*x_36 + x_9*x_36 + x_10*x_36 + x_15*x_36 + x_19*x_36 + x_23*x_36 + x_24*x_36\n"
                       "        + x_25*x_36 + x_26*x_36 + x_27*x_36 + x_35*x_36 + x_2*x_37 + x_7*x_37 + x_8*x_37 + x_9*x_37 + x_10*x_37 + x_11*x_37 + x_12*x_37 + x_13*x_37 + x_15*x_37 + x_16*x_37 + x_17*x_37 + x_20*x_37 + \n"
                       "        x_21*x_37 + x_22*x_37 + x_24*x_37 + x_25*x_37 + x_26*x_37 + x_29*x_37 + x_2*x_38 + x_3*x_38 + x_6*x_38 + x_7*x_38 + x_8*x_38 + x_12*x_38 + x_15*x_38 + x_16*x_38 + x_17*x_38 + x_18*x_38 + x_21*x_38\n"
                       "        + x_23*x_38 + x_24*x_38 + x_26*x_38 + x_27*x_38 + x_28*x_38 + x_29*x_38 + x_30*x_38 + x_31*x_38 + x_32*x_38 + x_33*x_38 + x_35*x_38 + x_6*x_39 + x_8*x_39 + x_9*x_39 + x_10*x_39 + x_11*x_39 + \n"
                       "        x_14*x_39 + x_16*x_39 + x_18*x_39 + x_19*x_39 + x_21*x_39 + x_22*x_39 + x_26*x_39 + x_27*x_39 + x_30*x_39 + x_33*x_39 + x_34*x_39 + x_1*x_40 + x_2*x_40 + x_3*x_40 + x_4*x_40 + x_5*x_40 + x_6*x_40 \n"
                       "        + x_8*x_40 + x_9*x_40 + x_11*x_40 + x_12*x_40 + x_13*x_40 + x_15*x_40 + x_16*x_40 + x_17*x_40 + x_20*x_40 + x_21*x_40 + x_22*x_40 + x_24*x_40 + x_25*x_40 + x_26*x_40 + x_31*x_40 + x_33*x_40 + \n"
                       "        x_34*x_40 + x_37*x_40 + x_38*x_40 + x_2*x_41 + x_4*x_41 + x_5*x_41 + x_12*x_41 + x_14*x_41 + x_19*x_41 + x_20*x_41 + x_21*x_41 + x_23*x_41 + x_26*x_41 + x_30*x_41 + x_31*x_41 + x_34*x_41 + \n"
                       "        x_39*x_41 + x_40*x_41 + x_2*x_42 + x_3*x_42 + x_5*x_42 + x_10*x_42 + x_11*x_42 + x_14*x_42 + x_15*x_42 + x_19*x_42 + x_24*x_42 + x_25*x_42 + x_26*x_42 + x_27*x_42 + x_29*x_42 + x_31*x_42 + \n"
                       "        x_33*x_42 + x_35*x_42 + x_36*x_42 + x_37*x_42 + x_38*x_42 + x_39*x_42 + x_6*x_43 + x_8*x_43 + x_9*x_43 + x_11*x_43 + x_12*x_43 + x_17*x_43 + x_19*x_43 + x_20*x_43 + x_21*x_43 + x_30*x_43 + \n"
                       "        x_31*x_43 + x_35*x_43 + x_36*x_43 + x_37*x_43 + x_39*x_43 + x_41*x_43 + x_1*x_44 + x_2*x_44 + x_3*x_44 + x_7*x_44 + x_8*x_44 + x_9*x_44 + x_14*x_44 + x_18*x_44 + x_22*x_44 + x_23*x_44 + x_25*x_44 \n"
                       "        + x_26*x_44 + x_27*x_44 + x_29*x_44 + x_31*x_44 + x_33*x_44 + x_35*x_44 + x_37*x_44 + x_39*x_44 + x_40*x_44 + x_41*x_44 + x_42*x_44 + x_1*x_45 + x_5*x_45 + x_6*x_45 + x_9*x_45 + x_13*x_45 + \n"
                       "        x_15*x_45 + x_18*x_45 + x_19*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + x_23*x_45 + x_24*x_45 + x_26*x_45 + x_27*x_45 + x_29*x_45 + x_32*x_45 + x_33*x_45 + x_34*x_45 + x_36*x_45 + x_42*x_45 + \n"
                       "        x_43*x_45 + x_1*x_46 + x_2*x_46 + x_4*x_46 + x_6*x_46 + x_7*x_46 + x_9*x_46 + x_10*x_46 + x_11*x_46 + x_13*x_46 + x_14*x_46 + x_22*x_46 + x_23*x_46 + x_26*x_46 + x_31*x_46 + x_32*x_46 + x_33*x_46 \n"
                       "        + x_39*x_46 + x_40*x_46 + x_41*x_46 + x_42*x_46 + x_43*x_46 + x_44*x_46 + x_45*x_46 + x_1*x_47 + x_2*x_47 + x_4*x_47 + x_6*x_47 + x_7*x_47 + x_8*x_47 + x_11*x_47 + x_12*x_47 + x_14*x_47 + \n"
                       "        x_15*x_47 + x_17*x_47 + x_18*x_47 + x_20*x_47 + x_22*x_47 + x_24*x_47 + x_26*x_47 + x_31*x_47 + x_34*x_47 + x_36*x_47 + x_37*x_47 + x_38*x_47 + x_39*x_47 + x_41*x_47 + x_43*x_47 + x_44*x_47 + \n"
                       "        x_45*x_47 + x_46*x_47 + x_1*x_48 + x_2*x_48 + x_3*x_48 + x_6*x_48 + x_10*x_48 + x_11*x_48 + x_12*x_48 + x_13*x_48 + x_15*x_48 + x_16*x_48 + x_17*x_48 + x_18*x_48 + x_19*x_48 + x_20*x_48 + \n"
                       "        x_27*x_48 + x_28*x_48 + x_31*x_48 + x_32*x_48 + x_41*x_48 + x_42*x_48 + x_1*x_49 + x_2*x_49 + x_5*x_49 + x_8*x_49 + x_9*x_49 + x_11*x_49 + x_12*x_49 + x_13*x_49 + x_14*x_49 + x_15*x_49 + x_16*x_49\n"
                       "        + x_17*x_49 + x_18*x_49 + x_21*x_49 + x_22*x_49 + x_25*x_49 + x_28*x_49 + x_30*x_49 + x_32*x_49 + x_36*x_49 + x_44*x_49 + x_45*x_49 + x_46*x_49 + x_47*x_49 + x_3 + x_4 + x_5 + x_6 + x_7 + x_8 + \n"
                       "        x_10 + x_12 + x_14 + x_15 + x_17 + x_18 + x_19 + x_21 + x_23 + x_28 + x_29 + x_31 + x_33 + x_37 + x_40 + x_43 + x_44 + x_45 + x_46 + x_47 + x_48,\n"
                       "    x_1*x_4 + x_1*x_5 + x_3*x_5 + x_1*x_6 + x_2*x_6 + x_3*x_6 + x_5*x_8 + x_6*x_8 + x_7*x_8 + x_2*x_9 + x_4*x_9 + x_6*x_9 + x_1*x_10 + x_3*x_10 + x_6*x_10 + x_7*x_10 + x_8*x_10 + x_9*x_10 + x_3*x_11 + \n"
                       "        x_4*x_11 + x_5*x_11 + x_7*x_11 + x_8*x_11 + x_10*x_11 + x_5*x_12 + x_8*x_12 + x_10*x_12 + x_11*x_12 + x_1*x_13 + x_5*x_13 + x_6*x_13 + x_7*x_13 + x_9*x_13 + x_11*x_13 + x_12*x_13 + x_1*x_14 + \n"
                       "        x_2*x_14 + x_6*x_14 + x_7*x_14 + x_9*x_14 + x_11*x_14 + x_12*x_14 + x_13*x_14 + x_4*x_15 + x_5*x_15 + x_6*x_15 + x_11*x_15 + x_14*x_15 + x_2*x_16 + x_3*x_16 + x_4*x_16 + x_5*x_16 + x_6*x_16 + \n"
                       "        x_7*x_16 + x_9*x_16 + x_10*x_16 + x_11*x_16 + x_12*x_16 + x_13*x_16 + x_14*x_16 + x_2*x_17 + x_3*x_17 + x_4*x_17 + x_5*x_17 + x_6*x_17 + x_8*x_17 + x_9*x_17 + x_10*x_17 + x_14*x_17 + x_16*x_17 + \n"
                       "        x_2*x_18 + x_6*x_18 + x_10*x_18 + x_12*x_18 + x_14*x_18 + x_15*x_18 + x_16*x_18 + x_1*x_19 + x_3*x_19 + x_6*x_19 + x_10*x_19 + x_11*x_19 + x_12*x_19 + x_13*x_19 + x_15*x_19 + x_16*x_19 + x_17*x_19\n"
                       "        + x_2*x_20 + x_3*x_20 + x_4*x_20 + x_5*x_20 + x_8*x_20 + x_10*x_20 + x_11*x_20 + x_12*x_20 + x_14*x_20 + x_15*x_20 + x_19*x_20 + x_2*x_21 + x_4*x_21 + x_5*x_21 + x_6*x_21 + x_7*x_21 + x_8*x_21 + \n"
                       "        x_9*x_21 + x_10*x_21 + x_12*x_21 + x_13*x_21 + x_15*x_21 + x_17*x_21 + x_18*x_21 + x_19*x_21 + x_1*x_22 + x_2*x_22 + x_3*x_22 + x_4*x_22 + x_7*x_22 + x_8*x_22 + x_11*x_22 + x_12*x_22 + x_13*x_22 +\n"
                       "        x_14*x_22 + x_15*x_22 + x_17*x_22 + x_20*x_22 + x_1*x_23 + x_3*x_23 + x_5*x_23 + x_6*x_23 + x_8*x_23 + x_10*x_23 + x_12*x_23 + x_13*x_23 + x_14*x_23 + x_16*x_23 + x_17*x_23 + x_19*x_23 + x_21*x_23\n"
                       "        + x_1*x_24 + x_2*x_24 + x_3*x_24 + x_4*x_24 + x_5*x_24 + x_7*x_24 + x_8*x_24 + x_15*x_24 + x_16*x_24 + x_18*x_24 + x_19*x_24 + x_20*x_24 + x_22*x_24 + x_23*x_24 + x_3*x_25 + x_4*x_25 + x_5*x_25 + \n"
                       "        x_7*x_25 + x_14*x_25 + x_21*x_25 + x_22*x_25 + x_24*x_25 + x_1*x_26 + x_3*x_26 + x_4*x_26 + x_5*x_26 + x_8*x_26 + x_10*x_26 + x_12*x_26 + x_13*x_26 + x_14*x_26 + x_15*x_26 + x_17*x_26 + x_18*x_26 \n"
                       "        + x_20*x_26 + x_21*x_26 + x_22*x_26 + x_24*x_26 + x_25*x_26 + x_1*x_27 + x_3*x_27 + x_9*x_27 + x_10*x_27 + x_12*x_27 + x_13*x_27 + x_14*x_27 + x_17*x_27 + x_19*x_27 + x_20*x_27 + x_21*x_27 + \n"
                       "        x_22*x_27 + x_23*x_27 + x_24*x_27 + x_25*x_27 + x_1*x_28 + x_2*x_28 + x_3*x_28 + x_4*x_28 + x_10*x_28 + x_12*x_28 + x_13*x_28 + x_14*x_28 + x_15*x_28 + x_16*x_28 + x_17*x_28 + x_19*x_28 + \n"
                       "        x_23*x_28 + x_1*x_29 + x_3*x_29 + x_4*x_29 + x_9*x_29 + x_13*x_29 + x_14*x_29 + x_15*x_29 + x_16*x_29 + x_17*x_29 + x_18*x_29 + x_19*x_29 + x_21*x_29 + x_24*x_29 + x_28*x_29 + x_1*x_30 + x_5*x_30 \n"
                       "        + x_9*x_30 + x_10*x_30 + x_11*x_30 + x_12*x_30 + x_13*x_30 + x_15*x_30 + x_17*x_30 + x_18*x_30 + x_20*x_30 + x_22*x_30 + x_24*x_30 + x_25*x_30 + x_26*x_30 + x_29*x_30 + x_1*x_31 + x_5*x_31 + \n"
                       "        x_6*x_31 + x_10*x_31 + x_11*x_31 + x_12*x_31 + x_13*x_31 + x_15*x_31 + x_19*x_31 + x_21*x_31 + x_23*x_31 + x_25*x_31 + x_27*x_31 + x_28*x_31 + x_29*x_31 + x_30*x_31 + x_1*x_32 + x_2*x_32 + \n"
                       "        x_6*x_32 + x_7*x_32 + x_8*x_32 + x_9*x_32 + x_10*x_32 + x_12*x_32 + x_14*x_32 + x_15*x_32 + x_16*x_32 + x_17*x_32 + x_19*x_32 + x_23*x_32 + x_24*x_32 + x_25*x_32 + x_26*x_32 + x_27*x_32 + \n"
                       "        x_31*x_32 + x_1*x_33 + x_2*x_33 + x_3*x_33 + x_5*x_33 + x_7*x_33 + x_9*x_33 + x_10*x_33 + x_12*x_33 + x_14*x_33 + x_16*x_33 + x_17*x_33 + x_19*x_33 + x_21*x_33 + x_22*x_33 + x_25*x_33 + x_31*x_33 \n"
                       "        + x_32*x_33 + x_1*x_34 + x_2*x_34 + x_3*x_34 + x_6*x_34 + x_7*x_34 + x_10*x_34 + x_12*x_34 + x_14*x_34 + x_16*x_34 + x_20*x_34 + x_21*x_34 + x_25*x_34 + x_27*x_34 + x_29*x_34 + x_30*x_34 + \n"
                       "        x_2*x_35 + x_5*x_35 + x_6*x_35 + x_9*x_35 + x_10*x_35 + x_12*x_35 + x_13*x_35 + x_14*x_35 + x_15*x_35 + x_19*x_35 + x_20*x_35 + x_22*x_35 + x_23*x_35 + x_25*x_35 + x_26*x_35 + x_29*x_35 + \n"
                       "        x_31*x_35 + x_32*x_35 + x_33*x_35 + x_34*x_35 + x_1*x_36 + x_2*x_36 + x_5*x_36 + x_9*x_36 + x_11*x_36 + x_12*x_36 + x_14*x_36 + x_22*x_36 + x_23*x_36 + x_25*x_36 + x_26*x_36 + x_28*x_36 + \n"
                       "        x_32*x_36 + x_4*x_37 + x_6*x_37 + x_7*x_37 + x_13*x_37 + x_16*x_37 + x_17*x_37 + x_19*x_37 + x_23*x_37 + x_26*x_37 + x_30*x_37 + x_34*x_37 + x_35*x_37 + x_3*x_38 + x_5*x_38 + x_6*x_38 + x_11*x_38 \n"
                       "        + x_12*x_38 + x_14*x_38 + x_16*x_38 + x_18*x_38 + x_19*x_38 + x_20*x_38 + x_21*x_38 + x_23*x_38 + x_24*x_38 + x_25*x_38 + x_27*x_38 + x_32*x_38 + x_33*x_38 + x_34*x_38 + x_35*x_38 + x_37*x_38 + \n"
                       "        x_1*x_39 + x_2*x_39 + x_3*x_39 + x_7*x_39 + x_9*x_39 + x_11*x_39 + x_12*x_39 + x_16*x_39 + x_24*x_39 + x_29*x_39 + x_31*x_39 + x_32*x_39 + x_35*x_39 + x_36*x_39 + x_38*x_39 + x_1*x_40 + x_2*x_40 +\n"
                       "        x_3*x_40 + x_5*x_40 + x_6*x_40 + x_9*x_40 + x_11*x_40 + x_19*x_40 + x_22*x_40 + x_25*x_40 + x_29*x_40 + x_32*x_40 + x_33*x_40 + x_35*x_40 + x_36*x_40 + x_37*x_40 + x_39*x_40 + x_1*x_41 + x_4*x_41 \n"
                       "        + x_6*x_41 + x_8*x_41 + x_13*x_41 + x_14*x_41 + x_15*x_41 + x_17*x_41 + x_18*x_41 + x_19*x_41 + x_22*x_41 + x_27*x_41 + x_28*x_41 + x_30*x_41 + x_32*x_41 + x_33*x_41 + x_37*x_41 + x_38*x_41 + \n"
                       "        x_39*x_41 + x_40*x_41 + x_2*x_42 + x_3*x_42 + x_4*x_42 + x_5*x_42 + x_6*x_42 + x_7*x_42 + x_9*x_42 + x_10*x_42 + x_14*x_42 + x_15*x_42 + x_20*x_42 + x_25*x_42 + x_28*x_42 + x_36*x_42 + x_39*x_42 +\n"
                       "        x_40*x_42 + x_41*x_42 + x_2*x_43 + x_3*x_43 + x_5*x_43 + x_7*x_43 + x_8*x_43 + x_10*x_43 + x_12*x_43 + x_13*x_43 + x_14*x_43 + x_15*x_43 + x_16*x_43 + x_18*x_43 + x_19*x_43 + x_21*x_43 + x_22*x_43\n"
                       "        + x_23*x_43 + x_33*x_43 + x_35*x_43 + x_37*x_43 + x_38*x_43 + x_39*x_43 + x_2*x_44 + x_3*x_44 + x_4*x_44 + x_10*x_44 + x_11*x_44 + x_12*x_44 + x_15*x_44 + x_16*x_44 + x_18*x_44 + x_19*x_44 + \n"
                       "        x_22*x_44 + x_24*x_44 + x_25*x_44 + x_26*x_44 + x_27*x_44 + x_28*x_44 + x_32*x_44 + x_37*x_44 + x_39*x_44 + x_41*x_44 + x_42*x_44 + x_43*x_44 + x_2*x_45 + x_3*x_45 + x_6*x_45 + x_9*x_45 + \n"
                       "        x_10*x_45 + x_12*x_45 + x_13*x_45 + x_14*x_45 + x_16*x_45 + x_17*x_45 + x_18*x_45 + x_19*x_45 + x_20*x_45 + x_21*x_45 + x_22*x_45 + x_26*x_45 + x_28*x_45 + x_30*x_45 + x_32*x_45 + x_33*x_45 + \n"
                       "        x_35*x_45 + x_36*x_45 + x_38*x_45 + x_39*x_45 + x_40*x_45 + x_41*x_45 + x_44*x_45 + x_1*x_46 + x_2*x_46 + x_12*x_46 + x_13*x_46 + x_15*x_46 + x_17*x_46 + x_20*x_46 + x_22*x_46 + x_23*x_46 + \n"
                       "        x_24*x_46 + x_25*x_46 + x_27*x_46 + x_28*x_46 + x_31*x_46 + x_34*x_46 + x_36*x_46 + x_40*x_46 + x_41*x_46 + x_2*x_47 + x_4*x_47 + x_6*x_47 + x_9*x_47 + x_13*x_47 + x_14*x_47 + x_15*x_47 + \n"
                       "        x_16*x_47 + x_17*x_47 + x_19*x_47 + x_20*x_47 + x_21*x_47 + x_25*x_47 + x_28*x_47 + x_30*x_47 + x_33*x_47 + x_37*x_47 + x_38*x_47 + x_40*x_47 + x_43*x_47 + x_44*x_47 + x_45*x_47 + x_46*x_47 + \n"
                       "        x_2*x_48 + x_4*x_48 + x_5*x_48 + x_6*x_48 + x_7*x_48 + x_10*x_48 + x_12*x_48 + x_16*x_48 + x_17*x_48 + x_19*x_48 + x_23*x_48 + x_26*x_48 + x_28*x_48 + x_31*x_48 + x_33*x_48 + x_34*x_48 + x_35*x_48\n"
                       "        + x_38*x_48 + x_41*x_48 + x_42*x_48 + x_45*x_48 + x_46*x_48 + x_1*x_49 + x_2*x_49 + x_4*x_49 + x_5*x_49 + x_7*x_49 + x_8*x_49 + x_10*x_49 + x_11*x_49 + x_13*x_49 + x_14*x_49 + x_18*x_49 + \n"
                       "        x_19*x_49 + x_23*x_49 + x_24*x_49 + x_25*x_49 + x_27*x_49 + x_29*x_49 + x_31*x_49 + x_32*x_49 + x_34*x_49 + x_35*x_49 + x_36*x_49 + x_40*x_49 + x_41*x_49 + x_46*x_49 + x_47*x_49 + x_1 + x_5 + x_7 \n"
                       "        + x_11 + x_14 + x_15 + x_18 + x_20 + x_21 + x_23 + x_26 + x_28 + x_31 + x_32 + x_33 + x_34 + x_37 + x_41 + x_42 + x_43 + x_44 + x_45 + x_46";

    vector<int> clear = generateRandomClear(50,49);

    vector<int> chi = {1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1,
                       1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0};

    angriff(publicKey, clear, ' ', chi);
}

int main() {

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    //angriffD3();

    //angriffD5();

    //angriffD7();

    angriffGruppe1();

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    cout << "Benötigte Zeit: \n";

    int min = chrono::duration_cast<chrono::minutes>(end - begin).count();;
    int sec = chrono::duration_cast<chrono::seconds>(end - begin).count();
    long msec = chrono::duration_cast<chrono::milliseconds>(end - begin).count();

    std::cout << min << ":" << sec%60 << ":" << msec%1000 << " [m:ss:ms]\n" << endl;

    return 0;
}
