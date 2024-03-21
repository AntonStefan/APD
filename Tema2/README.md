Anton Stefan, 331CC 

                    Tema2 -Apd

Tema pica teste respectiv pe LWL si unu de SITA, in rest
RR SI SQ trec integral.

Timp implementare: 
~10 ore

Detalii implementare:

Avem MyDispatcher, care asigneaza taskuri la diferite hosturi
sau noduri de proces, pe baza algoritmului selectat.

Algoritmul 1 Round Robin
-distribuie taskurile secvential hosturilor, circular, si am 
luat rrIndex care e atomic pentru a fi vizibil intre threaduri
si care tine cont de utlimul host caruia i-a fost asignat un task

Algoritmul 2 Shortest Queue
- asingneaza taskul hostului cu marimea cea mai mica a queului

Algoritmul 3 Size Interval Task Assignment
- distrubuie taskurile in functie de marime (short medium long)

Algoritmul 4 Least Work Left
- asigneaza taskul hostului cu cea mai putina munca ramasa de facut

Avem MyHost care primeste de la MyDispatcher
Incepe cu un queue initializat cu un comparator pentru a sorta dupa
prioritate, si cu 2 obiecte pentru sincronizari

Functia run
Verificam daca trebuie sa dam shutdown si toate taskurile
sunt incheiate, dupa daca taskul curent nu ruleaza, alegem
urmatorul task. Daca avem un task care ruleaza, luam taskDuration
care tine cont de durata taskului,  dam sleep pe ce durata are taskul,
calculam timpul ramas, daca timpul ramas e mai mic ca 0, incheiem
taskul curent , apoi daca este intrerupt, calculam la fel 
timpul ramas al taskului si verificam daca a ajuns mai mic ca 0 in acest caz
dandu-i finish, daca nu i s-a terminat timpul taskului, si e preemtabil, il adaugam 
inapoi in coada.

Functia addTask
Dam add la task, verificam daca e preemtabil si prioritatea
taskului urmator e mai mare, daca e punem flagul pe true, pentru a tine cont
in run, si dam interrupt.

Functia getWorkLeft
Calculeaza timpul total ramas adunand timpul tuturor taskurilor in coada si a
taskului curent.
