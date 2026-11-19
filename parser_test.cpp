#include "HTTPRequest.hpp"
#include <iostream>
#include <string>
#include <vector>

struct TestCase
{
    std::string name;
    std::string request;
    bool shouldSucceed;
};

int main()
{
    std::vector<std::string> methods;
    methods.push_back("GET");
    methods.push_back("POST");
    methods.push_back("DELETE");
    methods.push_back("PATCH");
    methods.push_back("TRACE");

    std::vector<std::string> protocols;
    protocols.push_back("HTTP/1.0");
    protocols.push_back("HTTP/1.1");
    protocols.push_back("HTTP/2");
    protocols.push_back("HTTP/3");

    std::vector<TestCase> tests;

    tests.push_back(TestCase());
    tests[0].name = "GET valide HTTP/1.1";
    tests[0].request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[0].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[1].name = "POST valide avec body";
    tests[1].request =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";
    tests[1].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[2].name = "HTTP/1.0 sans Host";
    tests[2].request =
        "GET /index.html HTTP/1.0\r\n"
        "\r\n";
    tests[2].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[3].name = "Host manquant en HTTP/1.1";
    tests[3].request =
        "GET / HTTP/1.1\r\n"
        "\r\n";
    tests[3].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[4].name = "Methode en minuscules";
    tests[4].request =
        "get / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[4].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[5].name = "Methode inconnue";
    tests[5].request =
        "UNKNOWN / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[5].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[6].name = "Path invalide";
    tests[6].request =
        "GET index.html HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[6].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[7].name = "Protocole invalide";
    tests[7].request =
        "GET / HTTP/9.9\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[7].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[8].name = "Ligne de requete avec champ supplementaire";
    tests[8].request =
        "GET / HTTP/1.1 extra\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[8].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[9].name = "Percent encoding valide";
    tests[9].request =
        "GET /file%20name HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[9].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[10].name = "Percent encoding invalide";
    tests[10].request =
        "GET /file%ZZ HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    tests[10].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[11].name = "Header sans deux-points";
    tests[11].request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "BrokenHeader\r\n"
        "\r\n";
    tests[11].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[12].name = "Nom de header vide";
    tests[12].request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        ": value\r\n"
        "\r\n";
    tests[12].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[13].name = "Nom de header avec espace";
    tests[13].request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content Length: 5\r\n"
        "\r\n";
    tests[13].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[14].name = "Header inconnu valide";
    tests[14].request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "X-Custom-Header: hello world\r\n"
        "\r\n";
    tests[14].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[15].name = "Valeur avec tabs";
    tests[15].request =
        "GET / HTTP/1.1\r\n"
        "Host:\tlocalhost\r\n"
        "\r\n";
    tests[15].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[16].name = "Content-Length identiques";
    tests[16].request =
        "POST / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "Content-Length: 5\r\n"
        "\r\n";
    tests[16].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[17].name = "Content-Length contradictoires";
    tests[17].request =
        "POST / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "Content-Length: 8\r\n"
        "\r\n";
    tests[17].shouldSucceed = false;

    tests.push_back(TestCase());
    tests[18].name = "Transfer-Encoding valide";
    tests[18].request =
        "POST / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n";
    tests[18].shouldSucceed = true;

    tests.push_back(TestCase());
    tests[19].name = "Host duplique";
    tests[19].request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Host: example.com\r\n"
        "\r\n";
    tests[19].shouldSucceed = false;

    int failures = 0;

    for (std::size_t i = 0; i < tests.size(); ++i)
    {
        std::cout << tests[i].name << ": ";

        try
        {
            HTTPRequest request(tests[i].request, methods, protocols);

            if (tests[i].shouldSucceed)
                std::cout << "OK\n";
            else
            {
                std::cout << "FAIL: accepte une requete invalide\n";
                ++failures;
            }
        }
        catch (const std::exception& error)
        {
            if (!tests[i].shouldSucceed)
                std::cout << "OK: " << error.what() << "\n";
            else
            {
                std::cout << "FAIL: " << error.what() << "\n";
                ++failures;
            }
        }
    }

    std::cout << "\nTests echoues: " << failures
        << " / " << tests.size() << "\n";

    return failures != 0;
}