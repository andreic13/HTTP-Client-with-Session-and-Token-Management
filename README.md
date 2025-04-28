# HTTP-Client-with-Session-and-Token-Management

Pentru rularea constanta a programului am creat functia 'run_server()' in care am implementat un loop ce: citeste de la stdin comanda, realizeaza conexiunea, apeleaza functia corespunzatoare (sau afiseaza mesaj de eroare daca nu e cunoscuta comanda), dupa care o incheie, intrucat protocolul HTTP este stateless.

1. register:
	Preiau username-ul si parola, verific sa nu contina spatii sau sa nu fie nule, dupa care creez obiectul JSON in care le includ prin biblioteca 'parson' si le transform in string. Creez pachetul de tip POST prin 'compute_post_request', oferind string-ul cu obiectul JSON drept payload (body_data) si il trimit catre server, dupa care primesc raspunsul. Afisez mesajul corespunzator si eliberez memoria alocata.

2. login:
	Este similar cu register, insa, in caz de succes, pe langa mesajul aferent, preiau cookie-ul de login. Pentru acesta am creat o variabila globala, 'cookie_for_connection'. Din raspunsul serverului, preiau inceputul cookie-ului si finalul acestuia. In caz ca a fost alocat anterior, il realoc pentru dimensiunea curenta sau, in caz, contrar il aloc. In continuare copiez informatia in variabila globala. De asemenea, am o variabila globala pentru token-ul jwt de acces catre biblioteca, pe care il eliberez aici daca a fost alocat anterior, deoarece nu vreau sa mai aiba acces utlizatorul dupa delogare.

3. enter_library:
	Initial, verific daca exista un cookie de conexiune (daca un utilizator s-a logat). Daca nu exista, afisez eroarea si inchid functia. Altfel, creez pachetul de tip GET prin functia 'compute_get_request' si il trimit catre server, dupa care primesc raspunsul. In caz de succes, pe langa afisarea mesajului, preiau token-ul din raspuns, realoc sau aloc, dupa caz, variabila globala pentru token, dupa care il copiez in aceasta.

4. get_books:
	Initial, verific daca exista un token de acces la biblioteca. Nu mai verific daca este logat deoarece, altfel, nu putea obtine token-ul. Daca nu exista, afisez eroarea si inchid functia. Trimit pachetul de get catre server si primesc raspunsul. Cand construiesc pachetul nu mai trimit si cookie-ul de login deoarece serverul verifica doar token-ul, ce nu putea fi obtinut fara cookie. In caz de succes, preiau cartile in format JSON prin functia implementata de mine 'basic_extract_json_response_list' in 'helpers', creez lista de obiecte JSON pe baza acestora, iar, pe rand, extrag fiecare obiect si afisez campurile sale in formatul impus.

5. get_book:
	Citesc id-ul cartii ce trebuie afisata de la stdin, sub forma de string, doarece, sub forma de int, ramaneam cu un \n ce duce la un mesaj de eroare deoarece nu era recunoscuta noua comanda. Fac asta inainte de a verifica token-ul, deoarece asa este descris in 'Exemplu sesiune'. Daca am token de acces, trimit mesajul de GET catre server la adresa corespunzatoare si astept raspunsul. In caz de succes, iau obiectul JSON prin functia 'basic_extract_json_response', preiau campurile din acesta si le afisez in formatul impus.

6. add_book:
	Citesc de la stdin, pe rand, toate campurile necesare, dupa care verific daca am vreun camp gol sau daca 'page_count' este un numar intreg. Fac asta inainte de a verifica token-ul, deoarece asa este cerut in checker. Daca este cazul, afisez un mesaj de eroare si inchei executia functiei. Altfel, construiesc obiectul JSON cu ajutorul bibliotecii 'parson', il transform in string, creez pachetul de tip POST si il trimit catre server. Preiau raspunsul si afisez mesajul corespunzator.

7. delete_book:
	Citesc id-ul cartii ce trebuie stearsa de la stdin. Daca am token de acces, construiesc mesajul de DELETE prin functia 'compute_delete_request' pe care am implementat-o mai simplu, tot in 'requests', il trimit catre server si primesc raspunsul. In functie de caz, afisez mesajul corespunzator.

8. logout:
	Daca utilizatorul nu are cookie de conexiune, primeste mesajul de eroare ce il informeaza ca nu este logat. Altfel, trimit mesajul de GET catre server si primesc raspunsul. In caz de succes, afisez mesajul corespunzator si eliberez cookie-ul de conexiune si token-ul jwt de acces catre biblioteca.

9. exit:
	In caz ca exita, eliberez cookie-ul de conexiune si token-ul jwt, dupa care ies din loop si se ajunge la comanda 'return' din 'main' ce inchide programul.

+ In functiile 'compute_get_request', 'compute_post_request' si 'compute_delete_request' am adaugat in pachet/mesaj, pe rand, campurile necesare, conform comentariilor din laborator. De asemenea, am adaugat ca parametru si in pachet token-ul jwt pentru acces la biblioteca, ca Header, conform enuntului. In plus, pentru 'Content-Length' corect, am creat corpul payload-ului adaugand caracterul '&' intre campurile acestuia, dupa care i-am preluat size-ul (realizat la indrumarea titularului de laborator).
