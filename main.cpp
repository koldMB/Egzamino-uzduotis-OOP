#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <regex>
#include <algorithm>
#include <cctype>

using namespace std;

// Funkcija nuorodų išrinkimui ir išvedimui
// Iš žodžių žemėlapio randa tinkamas nuorodas ir jas išveda į failą
void nuoroda(map<string, vector<int>>& wordPositions)
{
    // ----- Skaityti TLD sąrašą iš failo (nepriklausomai nuo raidžių dydžio) -----
    vector<string> tldList;
    // Atidaryti TLD sąrašo failą
    ifstream tldFile("nuorodos.txt");
    if (!tldFile) {
        cout << "Nepavyko atidaryti nuorodos.txt failo.\n";
        return;
    }
    // Skaityti kiekvieną TLD ir konvertuoti į mažąsias raides
    string line;
    while (getline(tldFile, line)) {
        if (line.empty()) continue;
        string tld = line;
        transform(tld.begin(), tld.end(), tld.begin(), ::tolower);
        tldList.push_back(tld);
    }
    tldFile.close();

    // Pagalbinė funkcija: pašalinti priekyje ir pabaigoje esančius skyrybos ženklus
    // Paliekamos URL dalys: . ir : jei jie naudojami URL (pvz. domenas.lt, port:8080)
    auto trimTrailingPunct = [](const string& s) -> string {
        if (s.empty()) return s;
        string result = s;
        
        // Pašalinti skyrybos ženklus iš pabaigos: . , ) : ; ! ?
        while (!result.empty()) {
            char c = result.back();
            if (c == '.' || c == ',' || c == ')' || c == ':' || c == ';' || c == '!' || c == '?' || c == ']') {
                result.pop_back();
            } else {
                break;
            }
        }
        
        // Pašalinti skyrybos ženklus iš pradžios: (
        while (!result.empty()) {
            char c = result.front();
            if (c == '(' || c =='[') {
                result = result.substr(1);
            } else {
                break;
            }
        }
        
        return result;
    };

    // Pagalbinė funkcija: patikrinti, ar žodis yra tinkama nuoroda (URL)
    auto isValidUrl = [&](const string& originalWord) -> bool {
        // Pašalinti skyrybos ženklus iš žodžio
        string word = trimTrailingPunct(originalWord);
        if (word.empty()) return false;

        // Rasti pagrindinio kompiuterio pavadinimo pradžią
        size_t start = 0;
        size_t schemePos = word.find("://");
        if (schemePos != string::npos)
            start = schemePos + 3;  // Prasideda po "://" (http://pvz.)
        else if (word.find("www.") == 0)
            start = 4;  // Prasideda po "www."

        // Rasti pagrindinio kompiuterio pavadinimo pabaigą (stabdyti ties '/', '?', '#' arba ':')
        size_t end = word.length();
        for (size_t i = start; i < word.length(); ++i) {
            char c = word[i];
            if (c == '/' || c == '?' || c == '#' || c == ':') {
                end = i;
                break;
            }
        }

        string hostname = word.substr(start, end - start);
        if (hostname.empty()) return false;

        // Tinkama nuoroda turi turėti bent vieną tašką pagrindiniam kompiuteriui (pvz. example.com)
        if (hostname.find('.') == string::npos)
            return false;

        // Konvertuoti pagrindinį kompiuterio pavadinimą į mažąsias raides
        string lowerHost = hostname;
        transform(lowerHost.begin(), lowerHost.end(), lowerHost.begin(), ::tolower);

        // Patikrinti, ar pagrindinis kompiuterio pavadinimas baigiasi žinomu TLD
        for (const string& tld : tldList) {
            if (lowerHost == tld) return true;               // Tikslus atitikimas (retai)
            string suffix = "." + tld;
            if (lowerHost.length() >= suffix.length() &&
                lowerHost.compare(lowerHost.length() - suffix.length(), suffix.length(), suffix) == 0)
                return true;
        }
        return false;
    };

    // ----- Išvesti tik tinkamas nuorodas (be pasikartojimų) -----
    ofstream output("nuorodosOutput.txt");
    if (!output) {
        cout << "Nepavyko sukurti isvesties failo.\n";
        return;
    }

    bool foundAny = false;
    set<string> outputtedUrls;  // Sekti unikalias jau išvestas nuorodas
    auto it = wordPositions.begin();
    while (it != wordPositions.end()) {
        if (isValidUrl(it->first)) {
            string trimmedUrl = trimTrailingPunct(it->first);
            // Išvesti tik jei ši nuoroda dar nebuvo išvesta
            if (outputtedUrls.find(trimmedUrl) == outputtedUrls.end()) {
                output << trimmedUrl << endl;
                outputtedUrls.insert(trimmedUrl);
                foundAny = true;
            }
            it = wordPositions.erase(it);
        } else {
            ++it;
        }
    }
    output.close();

    if (foundAny)
        cout << "Nuorodos išvestos į failą 'nuorodosOutput.txt'\n";
    else {
        cout << "Nerasta nuorodų.\n";
        remove("nuorodosOutput.txt");
    }
}

// Pagrindinė programa
int main()
{
    // Gauti failo pavadinimą iš vartotojo
    string filename;
    cout << "Failo pavadinimas: ";
    cin >> filename;

    // Atidaryti failą skaityti
    ifstream input(filename);

    if (!input)
    {
        cout << "Nepavyko atidaryti failo.\n";
        return 1;
    }

    // Nuskaityti žodžius iš failo ir saugoti jų padėtis
    map<string, vector<int>> wordPositions;

    string word;
    int index = 0;

    while (input >> word)
    {
        wordPositions[word].push_back(index);
        index++;
    }

    input.close();

    // Rodyti žodžius, kurie pasikartoja tekste
    cout << "\nPasikartojantys zodziai:\n";

    bool found = false;

    // Iteruoti per visus žodžius
    for (auto it = wordPositions.begin(); it != wordPositions.end(); it++)
    {
        // Jei žodis pasikartoja daugiau nei vieną kartą
        if (it->second.size() > 1)
        {
            found = true;

            cout << it->first << " : skaičius = " << it->second.size() << " | vietos = ";

            // Rodyti visas žodžio padėtis tekste
            for (size_t i = 0; i < it->second.size(); i++)
            {
                cout << it->second[i];
                if (i != it->second.size() - 1)
                    cout << ", ";
            }

            cout << '\n';
        }
    }

    if (!found)
    {
        cout << "Nėra pasikartojančių žodžių.\n";
    }
    
    // Išrinkti ir išvesti nuorodas
    nuoroda(wordPositions);
    cin.get();  // Laukti vartotojo įvesčių prieš baigiant 
    return 0;
}
