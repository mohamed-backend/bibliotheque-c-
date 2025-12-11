#include <iostream>           // Nécessaire pour std::cin, std::cout, std::cerr
#include <string>             // Nécessaire pour std::string, std::getline
#include <vector>             // Nécessaire pour std::vector
#include <unordered_map>      // Nécessaire pour std::unordered_map
#include <map>                // Nécessaire pour std::map
#include <memory>             // Nécessaire pour std::shared_ptr, std::make_shared
#include <algorithm>          // Nécessaire pour std::sort, std::find_if, std::remove_if
#include <limits>             // Nécessaire pour std::numeric_limits
#include <fstream>            // Nécessaire pour std::ifstream, std::ofstream
#include <filesystem>         // Nécessaire pour std::filesystem (C++17)
#include <functional>         // Nécessaire pour std::hash (hachage)
#include <sstream>            // Nécessaire pour std::stringstream
#include <iomanip>            // Nécessaire pour std::hex, std::setw, std::setfill

using namespace std;
namespace fs = std::filesystem;

// ==========================================
// FONCTIONS UTILITAIRES
// ==========================================
void viderBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ==========================================
// CLASSE UTILISATEUR 
// ==========================================
class Utilisateur {
private:
    string username;
    string passwordHash;
    string role; // "Client", "Admin", "SuperAdmin"

    // Fonction de hachage simple
    static string hashPassword(const string& password) {
        hash<string> hasher;
        size_t hashValue = hasher(password);
        
        stringstream ss;
        ss << hex << setw(16) << setfill('0') << hashValue;
        return ss.str();
    }

public:
    Utilisateur(const string& user, const string& pass, const string& r)
        : username(user), passwordHash(hashPassword(pass)), role(r) {}

    Utilisateur(const string& user, const string& hash, const string& r, bool isHash)
        : username(user), passwordHash(hash), role(r) {}

    string getUsername() const { return username; }
    string getPasswordHash() const { return passwordHash; }
    string getRole() const { return role; }

    bool checkPassword(const string& pass) const {
        return passwordHash == hashPassword(pass);
    }
};

// ==========================================
// GESTION DES UTILISATEURS
// ==========================================
class GestionUtilisateurs {
private:
    vector<Utilisateur> comptes;
    string fichierUtilisateurs = "utilisateurs.txt";

public:
    GestionUtilisateurs() {
        chargerUtilisateurs();
    }

    // Charger les utilisateurs depuis fichier
    void chargerUtilisateurs() {
        ifstream fichier(fichierUtilisateurs);
        if (!fichier) {
            // Créer des comptes par défaut si le fichier n'existe pas
            comptes = {
                Utilisateur("client", "123", "Client"),
                Utilisateur("admin", "456", "Admin"),
                Utilisateur("superadmin", "789", "SuperAdmin")
            };
            sauvegarderUtilisateurs();
            cout << ">> Fichier utilisateurs cree avec comptes par defaut" << endl;
            return;
        }

        string ligne;
        int count = 0;
        while (getline(fichier, ligne)) {
            if (ligne.empty()) continue;
            
            size_t pos1 = ligne.find(';');
            size_t pos2 = ligne.find(';', pos1 + 1);
            
            if (pos1 != string::npos && pos2 != string::npos) {
                string username = ligne.substr(0, pos1);
                string passwordHash = ligne.substr(pos1 + 1, pos2 - pos1 - 1);
                string role = ligne.substr(pos2 + 1);
                
                comptes.push_back(Utilisateur(username, passwordHash, role, true));
                count++;
            }
        }
        fichier.close();
        
        if (count > 0) {
            cout << ">> " << count << " utilisateurs charges" << endl;
        }
    }

    // Sauvegarder les utilisateurs dans fichier
    void sauvegarderUtilisateurs() {
        ofstream fichier(fichierUtilisateurs);
        for (const auto& user : comptes) {
            fichier << user.getUsername() << ";"
                    << user.getPasswordHash() << ";"
                    << user.getRole() << "\n";
        }
        fichier.close();
    }

    // Vérifier si un username existe déjà
    bool usernameExiste(const string& username) {
        for (const auto& user : comptes) {
            if (user.getUsername() == username) {
                return true;
            }
        }
        return false;
    }

    // Ajouter un nouvel utilisateur
    bool ajouterUtilisateur(const string& username, const string& password, const string& role) {
        if (usernameExiste(username)) {
            cout << ">> Erreur: Ce nom d'utilisateur existe deja!" << endl;
            return false;
        }
        
        if (role != "Client" && role != "Admin" && role != "SuperAdmin") {
            cout << ">> Erreur: Role invalide! Utilisez: Client, Admin ou SuperAdmin" << endl;
            return false;
        }
        
        if (password.length() < 3) {
            cout << ">> Erreur: Mot de passe trop court (minimum 3 caracteres)!" << endl;
            return false;
        }
        
        comptes.push_back(Utilisateur(username, password, role));
        sauvegarderUtilisateurs();
        cout << ">> Succes: Utilisateur '" << username << "' ajoute avec role '" << role << "'" << endl;
        return true;
    }

    // Supprimer un utilisateur
    bool supprimerUtilisateur(const string& username) {
        // Empêcher la suppression de son propre compte pendant la session
        // et du dernier SuperAdmin
        int nbSuperAdmin = 0;
        for (const auto& user : comptes) {
            if (user.getRole() == "SuperAdmin") nbSuperAdmin++;
        }
        
        for (auto it = comptes.begin(); it != comptes.end(); ++it) {
            if (it->getUsername() == username) {
                if (it->getRole() == "SuperAdmin" && nbSuperAdmin <= 1) {
                    cout << ">> Erreur: Impossible de supprimer le dernier SuperAdmin!" << endl;
                    return false;
                }
                comptes.erase(it);
                sauvegarderUtilisateurs();
                cout << ">> Succes: Utilisateur '" << username << "' supprime" << endl;
                return true;
            }
        }
        
        cout << ">> Erreur: Utilisateur non trouve!" << endl;
        return false;
    }

    // Lister tous les utilisateurs
    void listerUtilisateurs() {
        cout << "\n=== LISTE DES UTILISATEURS (" << comptes.size() << ") ===" << endl;
        cout << "=============================================" << endl;
        for (const auto& user : comptes) {
            cout << "Username: " << user.getUsername() 
                 << " | Role: " << user.getRole() 
                 << " | Hash: " << user.getPasswordHash().substr(0, 8) << "..." << endl;
        }
        cout << "=============================================" << endl;
    }

    // Changer le mot de passe d'un utilisateur
    bool changerMotDePasse(const string& username, const string& nouveauPassword) {
        for (auto& user : comptes) {
            if (user.getUsername() == username) {
                // Pour changer le mot de passe, on doit recréer l'utilisateur
                // Mais comme passwordHash est private, on utilise une approche différente
                // On supprime et recrée l'utilisateur
                string role = user.getRole();
                supprimerUtilisateur(username);
                ajouterUtilisateur(username, nouveauPassword, role);
                cout << ">> Mot de passe change pour '" << username << "'" << endl;
                return true;
            }
        }
        cout << ">> Erreur: Utilisateur non trouve!" << endl;
        return false;
    }

    // Getter pour la fonction login
    const vector<Utilisateur>& getComptes() const {
        return comptes;
    }

    // Authentifier un utilisateur
    shared_ptr<Utilisateur> authentifier(const string& username, const string& password) {
        for (const auto& user : comptes) {
            if (user.getUsername() == username && user.checkPassword(password)) {
                return make_shared<Utilisateur>(user);
            }
        }
        return nullptr;
    }
};

// ==========================================
// SYSTEME DE LOGIN AVEC INSCRIPTION
// ==========================================
shared_ptr<Utilisateur> login(GestionUtilisateurs& gestionUsers) {
    while (true) {
        cout << "\n=== SYSTEME D'AUTHENTIFICATION ===" << endl;
        cout << "1. Se connecter" << endl;
        cout << "2. Creer un nouveau compte (Client seulement)" << endl;
        cout << "3. Quitter le programme" << endl;
        cout << "==================================" << endl;
        cout << "Choix: ";
        
        int choix;
        if (!(cin >> choix)) {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            continue;
        }
        viderBuffer();

        if (choix == 1) {
            string user, pass;
            cout << "\n--- CONNEXION ---" << endl;
            cout << "Username: ";
            getline(cin, user);
            cout << "Password: ";
            getline(cin, pass);

            auto userAuth = gestionUsers.authentifier(user, pass);
            if (userAuth) {
                cout << "\n>> Connexion reussie!" << endl;
                cout << ">> Bienvenue " << userAuth->getUsername() 
                     << " (Role: " << userAuth->getRole() << ")" << endl;
                return userAuth;
            }
            cout << "\n>> Nom d'utilisateur ou mot de passe incorrect!" << endl;
        }
        else if (choix == 2) {
            cout << "\n=== CREATION DE COMPTE CLIENT ===" << endl;
            string user, pass, confPass;
            
            cout << "Nouveau username: ";
            getline(cin, user);
            
            if (gestionUsers.usernameExiste(user)) {
                cout << ">> Erreur: Ce nom d'utilisateur existe deja!" << endl;
                continue;
            }
            
            if (user.length() < 3) {
                cout << ">> Erreur: Username trop court (minimum 3 caracteres)!" << endl;
                continue;
            }
            
            cout << "Nouveau mot de passe: ";
            getline(cin, pass);
            cout << "Confirmer le mot de passe: ";
            getline(cin, confPass);
            
            if (pass != confPass) {
                cout << ">> Erreur: Les mots de passe ne correspondent pas!" << endl;
                continue;
            }
            
            if (pass.length() < 3) {
                cout << ">> Erreur: Le mot de passe doit contenir au moins 3 caracteres!" << endl;
                continue;
            }
            
            // Créer automatiquement un compte Client
            if (gestionUsers.ajouterUtilisateur(user, pass, "Client")) {
                cout << "\n>> Compte cree avec succes! Vous pouvez maintenant vous connecter." << endl;
            }
        }
        else if (choix == 3) {
            cout << "Au revoir!" << endl;
            exit(0);
        }
        else {
            cout << ">> Choix invalide!" << endl;
        }
    }
}

// ==========================================
// CLASSES MEDIA
// ==========================================

class Media {
protected:
    int id;
    string titre;
    bool dispo;

public:
    Media(int id, const string& titre, bool dispo) : id(id), titre(titre), dispo(dispo) {}
    virtual ~Media() = default;

    int getId() const { return id; }
    const string& getTitre() const { return titre; }
    bool isDispo() const { return dispo; }

    void emprunter() {
        if (dispo) {
            dispo = false;
            cout << ">> Succes: '" << titre << "' a ete emprunte." << endl;
        } else {
            cout << ">> Erreur: '" << titre << "' n'est pas disponible." << endl;
        }
    }

    void retourner() {
        dispo = true;
        cout << ">> Info: '" << titre << "' a ete retourne." << endl;
    }

    virtual void afficher(ostream& os) const = 0;
    virtual int getDureeMinutes() const { return 0; }
    virtual string getType() const = 0;

    friend ostream& operator<<(ostream& os, const Media& m) {
        m.afficher(os);
        return os;
    }
};

class Telechargeable {
protected:
    double tailleMo;
    string format;

public:
    Telechargeable(double tailleMo, const string& format) : tailleMo(tailleMo), format(format) {}
    virtual ~Telechargeable() = default;

    void afficherTelechargement(ostream& os) const {
        os << " [Fichier: " << format << " | " << tailleMo << " Mo]";
    }

    double getTailleMo() const { return tailleMo; }
    const string& getFormat() const { return format; }
};

class Livre : public virtual Media {
protected:
    string auteur;
    int nPage;

public:
    Livre(int id, const string& titre, bool dispo, const string& auteur, int nPage)
        : Media(id, titre, dispo), auteur(auteur), nPage(nPage) {}

    void afficher(ostream& os) const override {
        os << "[Livre] ID:" << id << " | " << titre
           << " | Auteur: " << auteur << " | " << nPage << "p"
           << " | Dispo: " << (dispo ? "Oui" : "Non");
    }
    string getType() const override { return "Livre"; }

    const string& getAuteur() const { return auteur; }
    int getNpage() const { return nPage; }
};

class Video : public virtual Media {
private:
    int duree;
    string qualite;

public:
    Video(int id, const string& titre, bool dispo, int duree, const string& qualite)
        : Media(id, titre, dispo), duree(duree), qualite(qualite) {}

    void afficher(ostream& os) const override {
        os << "[Video] ID:" << id << " | " << titre
           << " | Duree: " << duree << "min | " << qualite
           << " | Dispo: " << (dispo ? "Oui" : "Non");
    }
    int getDureeMinutes() const override { return duree; }
    string getType() const override { return "Video"; }

    const string& getQualite() const { return qualite; }
};

class Audio : public virtual Media {
protected:
    string publicateur;
    int duree;

public:
    Audio(int id, const string& titre, bool dispo, const string& publicateur, int duree)
        : Media(id, titre, dispo), publicateur(publicateur), duree(duree) {}

    void afficher(ostream& os) const override {
        os << "[Audio] ID:" << id << " | " << titre
           << " | Pub: " << publicateur << " | " << duree << "min"
           << " | Dispo: " << (dispo ? "Oui" : "Non");
    }
    int getDureeMinutes() const override { return duree; }
    string getType() const override { return "Audio"; }

    const string& getPublicateur() const { return publicateur; }
};

class Ebook : public Livre, public Telechargeable {
public:
    Ebook(int id, const string& titre, const string& auteur, int nPage, double tailleMo, const string& format)
        : Media(id, titre, true),
          Livre(id, titre, true, auteur, nPage),
          Telechargeable(tailleMo, format) {}

    void afficher(ostream& os) const override {
        Livre::afficher(os);
        Telechargeable::afficherTelechargement(os);
    }
    string getType() const override { return "Ebook"; }
};

class AudioBook : public Livre, public Audio {
public:
    AudioBook(int id, const string& titre, bool dispo, const string& auteur, int nPage, const string& publicateur, int duree)
        : Media(id, titre, dispo),
          Livre(id, titre, dispo, auteur, nPage),
          Audio(id, titre, dispo, publicateur, duree) {}

    void afficher(ostream& os) const override {
        os << "[AudioBook] ID:" << id << " | " << titre
           << " | Auteur: " << Livre::getAuteur()
           << " | Voix: " << Audio::getPublicateur()
           << " | Duree: " << Audio::getDureeMinutes() << "min"
           << " | Dispo: " << (dispo ? "Oui" : "Non");
    }
    string getType() const override { return "AudioBook"; }
};

// ==========================================
// BIBLIOTHEQUE
// ==========================================
class Bibliotheque {
private:
    vector<shared_ptr<Media>> catalogue;

public:
    void ajouterMedia(shared_ptr<Media> media) {
        catalogue.push_back(media);
    }

    void supprimerMedia(int id) {
        size_t before = catalogue.size();
        catalogue.erase(remove_if(catalogue.begin(), catalogue.end(),
                                  [id](const shared_ptr<Media>& media) { return media->getId() == id; }),
                        catalogue.end());
        size_t after = catalogue.size();

        if (after < before) {
            cout << ">> Media ID " << id << " supprime." << endl;
        } else {
            cout << ">> ID introuvable." << endl;
        }
    }

    void rechercherParTitre(const string& motCle) {
        cout << "\n--- Resultats Recherche : " << motCle << " ---" << endl;
        bool trouve = false;
        for (const auto& media : catalogue) {
            if (media->getTitre().find(motCle) != string::npos) {
                cout << *media << endl;
                trouve = true;
            }
        }
        if (!trouve) cout << "Aucun resultat." << endl;
    }

    void changerStatut(int id, bool emprunt) {
        auto it = find_if(catalogue.begin(), catalogue.end(), [id](const shared_ptr<Media>& media) { return media->getId() == id; });
        if (it != catalogue.end()) {
            if (emprunt) (*it)->emprunter();
            else (*it)->retourner();
        } else {
            cout << ">> Media introuvable." << endl;
        }
    }

    void afficherTout() {
        cout << "\n--- CATALOGUE COMPLET (" << catalogue.size() << " medias) ---" << endl;
        sort(catalogue.begin(), catalogue.end(),
             [](const shared_ptr<Media>& a, const shared_ptr<Media>& b) {
                 return a->getId() < b->getId();
             });

        for (const auto& media : catalogue) {
            cout << *media << endl;
        }
    }

    void afficherStatistiques() {
        int totalMedia = static_cast<int>(catalogue.size());
        int totalDuree = 0;
        int nbLivres = 0;
        int nbDispo = 0;

        for (const auto& media : catalogue) {
            totalDuree += media->getDureeMinutes();
            string type = media->getType();
            if (type == "Livre" || type == "Ebook" || type == "AudioBook") nbLivres++;
            if (media->isDispo()) nbDispo++;
        }

        cout << "\n--- STATISTIQUES ---" << endl;
        cout << "Nombre total de medias : " << totalMedia << endl;
        cout << "Medias disponibles : " << nbDispo << endl;
        cout << "Duree totale (Audio/Video) : " << totalDuree << " min" << endl;
        cout << "Nombre de livres (Papier/Ebook/AudioBook) : " << nbLivres << endl;
    }

    void sauvegarderDansFichier() {
        string nomFichier = "bibliotheque.txt";

        ofstream f(nomFichier);
        if (!f) {
            cerr << ">> ERREUR: Impossible d'ouvrir le fichier pour ecriture!" << endl;
            return;
        }

        for (const auto& media : catalogue) {
            f << media->getType() << ";"
              << media->getId() << ";"
              << media->getTitre() << ";"
              << (media->isDispo() ? "1" : "0");

            if (auto livre = dynamic_pointer_cast<Livre>(media)) {
                if (media->getType() != "AudioBook") {
                    f << ";" << livre->getAuteur() << ";" << livre->getNpage();
                }
            }

            if (auto video = dynamic_pointer_cast<Video>(media)) {
                f << ";" << video->getDureeMinutes() << ";" << video->getQualite();
            }

            if (auto audio = dynamic_pointer_cast<Audio>(media)) {
                f << ";" << audio->getPublicateur() << ";" << audio->getDureeMinutes();
            }

            if (auto ebook = dynamic_pointer_cast<Ebook>(media)) {
                f << ";" << ebook->getAuteur() << ";" << ebook->getNpage()
                  << ";" << ebook->getTailleMo() << ";" << ebook->getFormat();
            }

            if (auto audiobook = dynamic_pointer_cast<AudioBook>(media)) {
                f << ";" << audiobook->getAuteur() << ";" << audiobook->getNpage()
                  << ";" << audiobook->getPublicateur() << ";" << audiobook->getDureeMinutes();
            }

            f << "\n";
        }

        f.close();
        cout << ">> Catalogue sauvegarde: " << catalogue.size() << " medias" << endl;
    }

    void chargerDepuisFichier() {
        string nomFichier = "bibliotheque.txt";

        ifstream f(nomFichier);
        if (!f) {
            cout << ">> Info: Catalogue vide. Fichier '" << nomFichier << "' non trouve." << endl;
            return;
        }

        int count = 0;
        string ligne;
        while (getline(f, ligne)) {
            if (ligne.empty()) continue;

            vector<string> champs;
            size_t pos = 0;
            string token;
            while ((pos = ligne.find(';')) != string::npos) {
                token = ligne.substr(0, pos);
                champs.push_back(token);
                ligne.erase(0, pos + 1);
            }
            champs.push_back(ligne);

            if (champs.size() < 4) continue;

            string type = champs[0];
            int id = stoi(champs[1]);
            string titre = champs[2];
            bool dispo = (champs[3] == "1");

            if (type == "Livre" && champs.size() >= 6) {
                string auteur = champs[4];
                int pages = stoi(champs[5]);
                ajouterMedia(make_shared<Livre>(id, titre, dispo, auteur, pages));
                count++;
            } else if (type == "Video" && champs.size() >= 6) {
                int duree = stoi(champs[4]);
                string qualite = champs[5];
                ajouterMedia(make_shared<Video>(id, titre, dispo, duree, qualite));
                count++;
            } else if (type == "Audio" && champs.size() >= 6) {
                string pub = champs[4];
                int duree = stoi(champs[5]);
                ajouterMedia(make_shared<Audio>(id, titre, dispo, pub, duree));
                count++;
            } else if (type == "Ebook" && champs.size() >= 8) {
                string auteur = champs[4];
                int pages = stoi(champs[5]);
                double taille = stod(champs[6]);
                string format = champs[7];
                ajouterMedia(make_shared<Ebook>(id, titre, auteur, pages, taille, format));
                count++;
            } else if (type == "AudioBook" && champs.size() >= 8) {
                string auteur = champs[4];
                int pages = stoi(champs[5]);
                string pub = champs[6];
                int duree = stoi(champs[7]);
                ajouterMedia(make_shared<AudioBook>(id, titre, dispo, auteur, pages, pub, duree));
                count++;
            }
        }

        f.close();
        if (count > 0) {
            cout << ">> " << count << " medias charges depuis " << nomFichier << endl;
        }
    }

    void verifierFichier() {
        string nomFichier = "bibliotheque.txt";
        string cheminComplet = (fs::current_path() / nomFichier).string();

        cout << "\n--- VERIFICATION FICHIER ---" << endl;
        cout << "Emplacement: " << cheminComplet << endl;

        if (!fs::exists(cheminComplet)) {
            cout << ">> Le fichier n'existe pas!" << endl;
            return;
        }

        ifstream f(nomFichier);
        if (!f) {
            cout << ">> Impossible d'ouvrir le fichier en lecture!" << endl;
            return;
        }

        auto size = fs::file_size(cheminComplet);
        cout << "Taille: " << size << " octets" << endl;

        if (size == 0) {
            cout << ">> Fichier vide!" << endl;
            f.close();
            return;
        }

        cout << "\n--- CONTENU DU FICHIER ---" << endl;
        string ligne;
        int numLigne = 0;
        while (getline(f, ligne)) {
            numLigne++;
            cout << numLigne << ": " << ligne << endl;
        }

        f.close();
        cout << "--- FIN DU FICHIER ---" << endl;
        cout << "Total: " << numLigne << " lignes" << endl;
    }
};

// ==========================================
// MENU AJOUT MEDIA
// ==========================================
void menuAjouter(Bibliotheque& biblio) {
    int choixType, id, nPage = 0, duree = 0;
    double tailleMo = 0.0;
    string titre, auteur, format, qualite, pub;

    cout << "\n--- AJOUT DE MEDIA ---" << endl;
    cout << "1. Livre\n2. Video\n3. Audio\n4. Ebook\n5. AudioBook\nChoix : ";
    cin >> choixType;

    cout << "ID unique : "; cin >> id;
    viderBuffer();
    cout << "Titre : "; getline(cin, titre);

    switch (choixType) {
        case 1: {
            cout << "Auteur : "; getline(cin, auteur);
            cout << "Nombre de pages : "; cin >> nPage;
            biblio.ajouterMedia(make_shared<Livre>(id, titre, true, auteur, nPage));
            break;
        }
        case 2: {
            cout << "Duree (min) : "; cin >> duree;
            viderBuffer();
            cout << "Qualite : "; getline(cin, qualite);
            biblio.ajouterMedia(make_shared<Video>(id, titre, true, duree, qualite));
            break;
        }
        case 3: {
            viderBuffer();
            cout << "Publicateur : "; getline(cin, pub);
            cout << "Duree (min) : "; cin >> duree;
            biblio.ajouterMedia(make_shared<Audio>(id, titre, true, pub, duree));
            break;
        }
        case 4: {
            viderBuffer();
            cout << "Auteur : "; getline(cin, auteur);
            cout << "Nombre de pages : "; cin >> nPage;
            cout << "Taille (Mo) : "; cin >> tailleMo;
            viderBuffer();
            cout << "Format : "; getline(cin, format);
            biblio.ajouterMedia(make_shared<Ebook>(id, titre, auteur, nPage, tailleMo, format));
            break;
        }
        case 5: {
            viderBuffer();
            cout << "Auteur : "; getline(cin, auteur);
            cout << "Nombre de pages : "; cin >> nPage;
            viderBuffer();
            cout << "Narrateur : "; getline(cin, pub);
            cout << "Duree Audio (min) : "; cin >> duree;
            biblio.ajouterMedia(make_shared<AudioBook>(id, titre, true, auteur, nPage, pub, duree));
            break;
        }
        default:
            cout << "Type invalide." << endl;
            return;
    }
    cout << ">> Media ajoute avec succes." << endl;
}

// ==========================================
// MENU GESTION UTILISATEURS (SuperAdmin)
// ==========================================
void menuGestionUtilisateurs(GestionUtilisateurs& gestionUsers) {
    int choix = -1;
    
    while (choix != 0) {
        cout << "\n=== GESTION DES UTILISATEURS ===" << endl;
        cout << "1. Ajouter un nouvel utilisateur" << endl;
        cout << "2. Supprimer un utilisateur" << endl;
        cout << "3. Lister tous les utilisateurs" << endl;
        cout << "4. Changer mot de passe utilisateur" << endl;
        cout << "0. Retour au menu principal" << endl;
        cout << "Choix: ";
        
        if (!(cin >> choix)) {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }
        viderBuffer();

        switch (choix) {
            case 1: {
                string user, pass, role;
                cout << "\n--- AJOUT D'UTILISATEUR ---" << endl;
                cout << "Nouvel username: ";
                getline(cin, user);
                cout << "Mot de passe: ";
                getline(cin, pass);
                cout << "Role (Client/Admin/SuperAdmin): ";
                getline(cin, role);
                
                // Validation du rôle
                transform(role.begin(), role.end(), role.begin(), ::tolower);
                if (role == "client") role = "Client";
                else if (role == "admin") role = "Admin";
                else if (role == "superadmin") role = "SuperAdmin";
                
                if (role != "Client" && role != "Admin" && role != "SuperAdmin") {
                    cout << ">> Erreur: Role invalide! Utilisez: Client, Admin ou SuperAdmin" << endl;
                    break;
                }
                
                gestionUsers.ajouterUtilisateur(user, pass, role);
                break;
            }
            case 2: {
                string user;
                cout << "\n--- SUPPRESSION D'UTILISATEUR ---" << endl;
                cout << "Username a supprimer: ";
                getline(cin, user);
                gestionUsers.supprimerUtilisateur(user);
                break;
            }
            case 3:
                gestionUsers.listerUtilisateurs();
                break;
            case 4: {
                string user, newPass;
                cout << "\n--- CHANGEMENT DE MOT DE PASSE ---" << endl;
                cout << "Username: ";
                getline(cin, user);
                cout << "Nouveau mot de passe: ";
                getline(cin, newPass);
                gestionUsers.changerMotDePasse(user, newPass);
                break;
            }
            case 0:
                cout << ">> Retour au menu principal..." << endl;
                break;
            default:
                cout << ">> Choix invalide!" << endl;
        }
    }
}

// ==========================================
// MENUS PAR ROLE
// ==========================================
void montrerMenuClient() {
    Bibliotheque biblio;
    int choix = -1;

    cout << "\n===================================" << endl;
    cout << "  BIBLIOTHEQUE MULTIMEDIA (CLIENT)" << endl;
    cout << "===================================" << endl;

    biblio.chargerDepuisFichier();

    while (choix != 0) {
        cout << "\n--- MENU CLIENT ---" << endl;
        cout << "1. Afficher tout le catalogue" << endl;
        cout << "2. Rechercher un media" << endl;
        cout << "3. Emprunter un media" << endl;
        cout << "4. Retourner un media" << endl;
        cout << "0. Deconnexion" << endl;
        cout << "Votre choix : ";

        if (!(cin >> choix)) {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }

        switch (choix) {
            case 1: 
                biblio.afficherTout(); 
                break;
            case 2: {
                string motCle;
                cout << "Mot du titre : ";
                viderBuffer();
                getline(cin, motCle);
                biblio.rechercherParTitre(motCle);
                break;
            }
            case 3: {
                int id;
                cout << "ID du media a emprunter : "; 
                cin >> id;
                biblio.changerStatut(id, true);
                break;
            }
            case 4: {
                int id;
                cout << "ID du media a retourner : "; 
                cin >> id;
                biblio.changerStatut(id, false);
                break;
            }
            case 0:
                biblio.sauvegarderDansFichier();
                cout << "\n>> Deconnexion..." << endl;
                break;
            default:
                cout << ">> Choix invalide!" << endl;
                viderBuffer();
        }
    }
}

void montrerMenuAdmin() {
    Bibliotheque biblio;
    int choix = -1;

    cout << "\n===================================" << endl;
    cout << "  BIBLIOTHEQUE MULTIMEDIA (ADMIN)" << endl;
    cout << "===================================" << endl;

    biblio.chargerDepuisFichier();

    while (choix != 0) {
        cout << "\n--- MENU ADMINISTRATEUR ---" << endl;
        cout << "1. Afficher tout le catalogue" << endl;
        cout << "2. Ajouter un media" << endl;
        cout << "3. Rechercher un media" << endl;
        cout << "4. Emprunter / Retourner" << endl;
        cout << "5. Supprimer un media" << endl;
        cout << "6. Voir les statistiques" << endl;
        cout << "0. Deconnexion" << endl;
        cout << "Votre choix : ";

        if (!(cin >> choix)) {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }

        switch (choix) {
            case 1: biblio.afficherTout(); break;
            case 2: menuAjouter(biblio); break;
            case 3: {
                string motCle;
                cout << "Mot du titre : ";
                viderBuffer();
                getline(cin, motCle);
                               biblio.rechercherParTitre(motCle);
                break;
            }
            case 4: {
                int id, action;
                cout << "ID du media : "; 
                cin >> id;
                cout << "1. Emprunter\n2. Retourner\nChoix : "; 
                cin >> action;
                biblio.changerStatut(id, (action == 1));
                break;
            }
            case 5: {
                int id;
                cout << "ID a supprimer : "; 
                cin >> id;
                biblio.supprimerMedia(id);
                break;
            }
            case 6: 
                biblio.afficherStatistiques(); 
                break;
            case 0:
                biblio.sauvegarderDansFichier();
                cout << "\n>> Deconnexion..." << endl;
                break;
            default:
                cout << ">> Choix invalide!" << endl;
                viderBuffer();
        }
    }
}

void montrerMenuSuperAdmin(GestionUtilisateurs& gestionUsers) {
    Bibliotheque biblio;
    int choix = -1;

    cout << "\n===================================" << endl;
    cout << "  BIBLIOTHEQUE MULTIMEDIA (SUPER ADMIN)" << endl;
    cout << "===================================" << endl;
    cout << "Repertoire courant: " << fs::current_path() << endl;

    biblio.chargerDepuisFichier();

    while (choix != 0) {
        cout << "\n--- MENU SUPER ADMINISTRATEUR ---" << endl;
        cout << "1. Afficher tout le catalogue" << endl;
        cout << "2. Ajouter un media" << endl;
        cout << "3. Rechercher un media" << endl;
        cout << "4. Emprunter / Retourner" << endl;
        cout << "5. Supprimer un media" << endl;
        cout << "6. Voir les statistiques" << endl;
        cout << "7. Verifier le fichier de sauvegarde" << endl;
        cout << "8. Gestion des utilisateurs" << endl;
        cout << "0. Deconnexion" << endl;
        cout << "Votre choix : ";

        if (!(cin >> choix)) {
            cin.clear();
            viderBuffer();
            cout << ">> Choix invalide!" << endl;
            choix = -1;
            continue;
        }

        switch (choix) {
            case 1: 
                biblio.afficherTout(); 
                break;
            case 2: 
                menuAjouter(biblio); 
                break;
            case 3: {
                string motCle;
                cout << "Mot du titre : ";
                viderBuffer();
                getline(cin, motCle);
                biblio.rechercherParTitre(motCle);
                break;
            }
            case 4: {
                int id, action;
                cout << "ID du media : "; 
                cin >> id;
                cout << "1. Emprunter\n2. Retourner\nChoix : "; 
                cin >> action;
                biblio.changerStatut(id, (action == 1));
                break;
            }
            case 5: {
                int id;
                cout << "ID a supprimer : "; 
                cin >> id;
                biblio.supprimerMedia(id);
                break;
            }
            case 6: 
                biblio.afficherStatistiques(); 
                break;
            case 7: 
                biblio.verifierFichier(); 
                break;
            case 8: 
                menuGestionUtilisateurs(gestionUsers); 
                break;
            case 0:
                biblio.sauvegarderDansFichier();
                gestionUsers.sauvegarderUtilisateurs();
                cout << "\n>> Deconnexion..." << endl;
                break;
            default:
                cout << ">> Choix invalide!" << endl;
                viderBuffer();
        }
    }
}

// ==========================================
// FONCTION PRINCIPALE
// ==========================================
int main() {
    cout << "==============================================" << endl;
    cout << "  SYSTEME DE GESTION DE BIBLIOTHEQUE V2.0" << endl;
    cout << "==============================================" << endl;
    cout << "Dossier de travail: " << fs::current_path() << endl;
    
    // Initialiser la gestion des utilisateurs
    GestionUtilisateurs gestionUsers;
    
    // Boucle principale pour permettre de se reconnecter après déconnexion
    while (true) {
        cout << "\n=== ACCUEIL ===" << endl;
        
        // Login avec option de création de compte
        auto user = login(gestionUsers);
        string role = user->getRole();
        
        // Afficher le menu selon le rôle
        if (role == "Client") {
            montrerMenuClient();
        }
        else if (role == "Admin") {
            montrerMenuAdmin();
        }
        else if (role == "SuperAdmin") {
            montrerMenuSuperAdmin(gestionUsers);
        }
        
        cout << "\n>> Retour a l'ecran d'accueil..." << endl;
    }
    
    return 0;
}
