# tema1
Anton Stefan, 331CC
                                        Tema 1 APD

Detalii tema:

Ceritna:
Tema a constat in implementarea paralela a algoritmului Maching Squares folosind 
Pthreads pornind de la implementarea secventiala a acesteia.

---------------------------------------------------------------------------------
Rulare:
./tema1_par <fisier_de_intrare> <fisier_de_iesire> <nr_threaduri>

---------------------------------------------------------------------------------
Timp implementare: Undeva la 17 ore

---------------------------------------------------------------------------------
Detalii implementare:

Pentru a trece testele au fost paralizate functiile rescale_iamge, sample_grid si
march.

Pentru a transmite datele fiecarui thread am folosit o structura definita 
thread_data. Am inceput programul in main prin calcularea numarului de threaduri 
din argumentul dat. Apoi am rezervat memorie pentru numarul de threaduri in cauza 
si pentru datele structurii thread_data. Am definit bariera si apoi am initializat-o.
Se citeste imaginea image. Initializam conturul, iar dupa definim noua imagine scalata 
dupa dimensiunile cerute in enunt. Apoi rezervam memorie pentru grid.


Dupa punem datele in structura thread_data corespunzatoare fiecarui thread
si cream threadul respectiv.

thread_combined_work():

In thread_combined_work() calculam startul si sfarsitul pentru threadul in cauza iar apoi 
apelam functiile thread_rescale_image_section(), thread_sample() si thread_march() cu bariera 
intre ele pentru sincronizarea threadurilor.


thread_rescale_image_section():

am tranmis ca parametrii idul threadului si numarul total de threaduri pentru a calcula local 
inceputul si sfarsitul threadului. Dupa am luat implementarea de interpolare bicubica si 
am schimbat ca datele sa fie puse in imaginea destinatie in loc de cea initiala.


thread_sample():
primul pas al algoritmului,
am transmis ca parametrii startrow si endrow calculate in thread_combined_work
pentru paralizarea primului for iar pentru al doilea for am calculat local newstartrow 
si newendrow pentru directia y

construieste un grid de valori binare in functie de sigma si operatiile sunt executate
intr-un range specific de randuri (fiecarui thread) pentru procesarea paralela.


thread_march():
al doilea pas al algoritmului, 
am transmis ca parametrii startrow si endrow calculate in thread_combined_work
pentru implementarea multi-threading. Identifica ce tip de contur corespunde pentru 
fiecare subgrid si determina valorile binare pentru fragmentele sample si inlocuieste 
pixelii cu pixelii de contur


Dupa ce am terminat toate operatiile dam join la threaduri. Scriem imaginea, eliberam 
resursele si dam destroy la bariera.









