#include <iostream>
#include <memory>
#include <string>
using namespace std;

class media
{
protected:
    int id;
    string titre;
    bool dispo;

public:
    media(int h, string w, bool j) : id(h), titre(w), dispo(j) {}

    virtual ~media()
    {
        cout << "deleted" << endl;
        
        
    }

    virtual void afficher()  = 0;
};

class livre : public media
{
private:
    string auteur;
    int Npage;

public:
    livre(int id, string titre, bool dispo, string auteur, int Npage)
        : media(id, titre, dispo), auteur(auteur), Npage(Npage) {}

    void afficher()  override
    {
        cout << "Le livre \"" << titre
             << "\" de l'auteur " << auteur
             << " contient " << Npage << " pages."
             << endl;
    }
};
class video : public media
{
private:
    int dure;
    string qualit;

public:
    video(int id, string titre, bool dispo, int dure, string qualit) 
    : media(id, titre, dispo), dure(dure), qualit(qualit) {}
    void afficher()  override
    {
        cout << "Le video \"" << titre
             << "\" de dure " << dure 
             << " est de qualite " << qualit<< endl;
    }
};
class audio : public media {
private:
    string publicateur;
    int duree;
public:
    
    audio(int t,  string p, bool a,  string art, int d)
        : media(t, p, a), publicateur(art), duree(d) {}

    void afficher() override {
        cout << "Audio: " << titre <<  ", publie par " << publicateur <<  ", duree: " << duree << endl;
    }
};

int main()
{
    cout << "========TEST========="<<endl;
    cout << "---- TEST LIVRE ----" << endl;
    media* m1 = new livre(1, "Harry Potter", true, "J.K. Rowling", 350);
    m1->afficher();
    delete m1;

    cout << "\n---- TEST VIDEO ----" << endl;
    media* m2 = new video(2, "Avengers", true, 120, "4K");
    m2->afficher();
    delete m2;

    cout << "\n---- TEST AUDIO ----" << endl;
    media* m3 = new audio(3, "Podcast Tech", true, "Houssam Studio", 45);
    m3->afficher();
    delete m3;

    cout << "\n---- FIN DES TESTS ----" << endl;

    return 0;
}