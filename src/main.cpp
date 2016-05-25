#include <SFML/Graphics.hpp>
#include "dirent.h"
#include "Histogram.hpp"


std::vector<std::string> openDir(const std::string& dirName, bool excludeDirs = false) {
    std::vector<std::string> names;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirName.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string n(ent->d_name);
            if (excludeDirs && (n == "." || n == ".."))
                continue;
            names.push_back(std::move(n));
        }
        closedir(dir);
    }
    else
        printf("could not open directory \"%s\"\n", dirName.c_str());

    return names;
}


int main(void) {
    const std::string dir("res/jpg/");
    auto fileList = openDir(dir, true);

    Histogram h;
    //h.saveProfileToFile("profiles/default.png");
    /*for (auto& file : fileList) {
        printf("analyzing file %s%s...\n", dir.c_str(), file.c_str());
        h.analyzeFile(dir+file);
    }

    h.createProfile();
    h.saveProfileToFile("profiles/dataset1.png");*/
    //h.loadProfileFromFile("profiles/dataset1.png");
    h.analyzeFile("res/MonaLisa.png");
    h.writeToFile("MonaLisaHistogram.png");

    return 0;
}
