#include <iostream>
#include <string>
using namespace std;

class media {
protected:
    int id;
    string titre;
    bool dispo;

public:
    media(int h, string w, bool j) : id(h), titre(w), dispo(j) {}
    virtual ~media() { cout << "deleted" << endl; }
    virtual void afficher() = 0;
};
//===== TELECHARGEABLE =====//
class Telechargeable {
protected:
    double tailleMo; // Taille en Mégaoctets
    string format;   // ex: MP3, PDF, MP4

public:
    Telechargeable(double taille, string fmt) : tailleMo(taille), format(fmt) {}

    void afficherTelechargement() const {
        cout << "[Info Fichier] Format: " << format 
             << " | Taille: " << tailleMo << " Mo" << endl;
    }
};

// ===== LIVRE =====
class livre : public virtual media {
protected:
    string auteur;
    int Npage;

public:
    livre(int id, string titre, bool dispo, string auteur, int Npage)
        : media(id, titre, dispo), auteur(auteur), Npage(Npage) {}

    void afficher() override {
        cout << "Livre: " << titre 
             << ", Auteur: " << auteur
             << ", Pages: " << Npage << endl;
    }
};

// ===== VIDEO =====
class video : public media {
private:
    int dure;
    string qualit;

public:
    video(int id, string titre, bool dispo, int dure, string qualit)
        : media(id, titre, dispo), dure(dure), qualit(qualit) {}

    void afficher() override {
        cout << "Video: " << titre 
             << ", Durée: " << dure 
             << ", Qualité: " << qualit << endl;
    }
};

// ===== AUDIO =====
class audio : public virtual media {
protected:
    string publicateur;
    int duree;

public:
    audio(int id, string titre, bool dispo, string publicateur, int duree)
        : media(id, titre, dispo), publicateur(publicateur), duree(duree) {}

    void afficher() override {
        cout << "Audio: " << titre 
             << ", Publié par: " << publicateur
             << ", Durée: " << duree << endl;
    }
};
class Ebook : public livre, public Telechargeable {
Ebook(int id, string titre, string auteur, int Npage, double taille, string format)
        : media(id, titre, true), // Init du grand-parent
          livre(id, titre, true, auteur, Npage), // Init du parent nature
          Telechargeable(taille, format) {} // Init de la caractéristique

    void afficher() override {
        cout << "--- EBOOK ---" << endl;
        livre::afficher();           // Affiche les infos livre
        afficherTelechargement();    // Affiche les infos fichier
    }

};

// ===== AUDIOBOOK =====
class audiobook : public livre, public audio {
public:
    audiobook(int id, string titre, bool dispo,
              string auteur, int Npage,
              string publicateur, int duree)
        : media(id, titre, dispo),   // call media ONCE
          livre(id, titre, dispo, auteur, Npage),
          audio(id, titre, dispo, publicateur, duree) {}

    void afficher() override {
        cout << "AUDIOBOOK" << endl;
        cout << "- Titre: " << titre << endl;
        cout << "- Auteur: " << auteur << endl;
        cout << "- Pages: " << Npage << endl;
        cout << "- Publié par: " << publicateur << endl;
        cout << "- Durée Audio: " << duree << endl;
    }
};

int main() {
    cout << "========TEST=========\n";

    cout << "---- TEST LIVRE ----\n";
    media* m1 = new livre(1, "Harry Potter", true, "J.K. Rowling", 350);
    m1->afficher();
    delete m1;

    cout << "\n---- TEST VIDEO ----\n";
    media* m2 = new video(2, "Avengers", true, 120, "4K");
    m2->afficher();
    delete m2;

    cout << "\n---- TEST AUDIO ----\n";
    media* m3 = new audio(3, "Podcast Tech", true, "Houssam Studio", 45);
    m3->afficher();
    delete m3;

    cout << "\n---- TEST AUDIOBOOK ----\n";
    media* m4 = new audiobook(4, "LOTR", true, "Tolkien", 500, "StudioX", 120);
    m4->afficher();
    delete m4;

    return 0;
}
