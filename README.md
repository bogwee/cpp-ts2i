# Projet de Transcodage : Python → C++

## 1. Contexte du projet  
L’objectif de ce projet est de **transcoder** un programme Python existant (codec d’images utilisant une DCT simplifiée) vers **C++** en respectant une structure plus modulaire avec des classes et en utilisant des bibliothèques adaptées pour le traitement d’images et l’algèbre linéaire.  

Le code Python d’origine utilisait :
- **NumPy** pour les matrices et calculs,
- **PIL (Pillow)** et **Matplotlib** pour le chargement et l’affichage des images.  

En C++, nous avons remplacé ces bibliothèques par :
- **OpenCV** pour le chargement, la manipulation et la sauvegarde d’images,  
- **Eigen** pour les calculs matriciels et la DCT,  
- **CMake** pour faciliter la compilation et la gestion des dépendances.

---

## 2. Approche de transcodage  

1. **Analyse du code Python** :  
   - Identification des étapes clés : lecture d’image, compression par DCT bloc 8×8, quantification, masquage des fréquences hautes, décompression inverse et affichage.  
   - Séparation en fonctions claires (`compressU`, `decompressU`, etc.) pour les transcrire en méthodes C++.  

2. **Traduction en C++** :
   - Les tableaux NumPy sont remplacés par des `Eigen::MatrixXd` pour faciliter les opérations matricielles.  
   - Les images sont traitées via `cv::Mat` d’OpenCV, en séparant les canaux BGR et en appliquant la DCT sur chaque canal.  
   - Une structure modulaire :  
     - **Fonctions utilitaires** pour DCT, quantification, et masquage.  
     - **Boucles bloc-par-bloc** pour la compression et la décompression, comme dans le code Python.  

3. **Résultat final** :  
   - Un exécutable C++ qui prend une image en entrée, applique la compression et sauvegarde l’image décompressée.  
   - Affiche aussi le **taux de compression** approximatif.  

---

## 3. Librairies à installer  

Sur Ubuntu/Debian, ouvrez un terminal et exécutez :  
```bash
sudo apt update
sudo apt install -y build-essential cmake libopencv-dev libeigen3-dev
