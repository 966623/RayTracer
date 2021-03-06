#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Vec3.hpp"
#include "ImgSettings.hpp"
#include "Camera.hpp"
#include "Image.hpp"
#include "Object.hpp"
#include "Sphere.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "Ellipsoid.hpp"
#include "Phong.hpp"
#include "Light.hpp"
#include "Basiclight.hpp"
#include "Spotlight.hpp"
#include "Polygon.hpp"
#include "Triangle.hpp"
#include "Vertex.hpp"
#include "UvCoord.hpp"
#include "Normal.hpp"
#include "Scene.hpp"

using namespace std;

void readFile(char* fileName, ifstream& stream);
ImgSettings parseFile(ifstream& inputFile);
void makeImage(ImgSettings settings);
int makeObjects(ImgSettings& settings, Camera &c, Image &i, vector<Object*> &obj, vector<Vertex*> &vert, vector<Normal*> &norm, vector<UvCoord*> &uvs, vector<Polygon*> &poly, vector<Material*> &mtl, vector<Texture *> tex, vector<Light*> &lt);
void render(Image &img, Camera &cam, vector<Object*> &obj, vector<Vertex*> &vert, vector<Normal*> &norm, vector<UvCoord*> &uvs, vector<Polygon*> &poly, vector<Material*> &mtl, vector<Texture *> tex, vector<Light*> &lt, ofstream &file);
void errorExit(void);
ofstream outputFile;
char *inputFileName;
int main(int argc, char **argv)
{
    //
    atexit(errorExit);
    // Check to see that there is an input file
    if(argc != 2){
        cout << "Invalid nubmer of arguments\n";
        return 0;
    }

    // Read file into a stream
    ifstream inputFile;
    inputFileName = argv[1];
    readFile(argv[1], inputFile);

    // Parse file into image settings
    ImgSettings settings = parseFile(inputFile);
    cout << settings.to_str();
    inputFile.close();

    // Output image
    makeImage(settings);
    
    return 0;
}

ImgSettings parseFile(ifstream& inputFile){
    string line;
    ImgSettings newSettings;
    cout << "Parsing File...\n";
    //Iterate over eachline
    do{
        getline (inputFile, line);
        vector<string> propList;
        if(line.size() == 0){
            //do nothing, empty line
        }
        else if(line[0] == '#'){
            // Comment found, do nothing
        }
        else{ // settings found, add a new setting
            char lineChar[line.size()+1];
            strcpy(lineChar, line.c_str());
            char * token;
            char * name;
            name = strtok (lineChar," \t");
            token = strtok (NULL," \t");

            //Put properties on this line into a vector
            while (token != NULL)
            {
                propList.push_back(token);
                token = strtok (NULL, " \t");
            }
            newSettings.setData(name, propList);
            
        }

    } while(inputFile.tellg() != -1);
    return newSettings;
}


int makeObjects(ImgSettings& settings, Camera &c, Image &i, 
        vector<Object*> &obj, vector<Vertex*> &vert, vector<Normal*> &norm, vector<UvCoord*> &uvs, vector<Polygon*> &poly, 
        vector<Material*> &mtl, vector<Texture *> tex, vector<Light*> &lt){

    //IMAGE -----
    cout << "Setting Up Image...\n";
    // Image size
    vector<string> imsize = settings.getData("imsize");

    // check validity
    if(imsize.size() != 2){
        cout << "ERROR: IMAGE SIZE WITH 2 DIMENSIONS MUST BE PROVIDED\n";
        exit(EXIT_FAILURE);
    }

    float width = stof(imsize[0]);
    float height = stof(imsize[1]);
    

    if(width < 0 || height < 0){
        cout << "ERROR: IMAGE DIMENSIONS MUST BE POSITIVE\n";
        exit(EXIT_FAILURE);
    }

    float ratio = width / height;

    //background color
    cout << "Image size : " << width << " x " << height << "\n";
    vector<string> bkgcolor = settings.getData("bkgcolor");

    // create new Image object;
    Image newImg = Image(width, height, bkgcolor);
    i = newImg;
    // ----------------

    //CAMERA -----
    cout << "Setting Up Camera...\n";
    vector<string> eyeLoc = settings.getData("eye");
    vector<string> updir = settings.getData("updir");
    vector<string> viewdir = settings.getData("viewdir");
    vector<string> fov = settings.getData("fovv");

    // Create new camera object
    Camera mainCam = Camera(ratio, eyeLoc, updir, viewdir, fov);

    if(settings.seek("parallel") != -1){
        cout << "Camera uses parallel projection\n";
        mainCam.setPerspective(false);
    }
    
    c = mainCam;
    // -----------

    // Objects ------
    cout << "Setting up objects...\n";

    // find first material
    cout << "\rMaking Material";
    settings.seekStart();
    settings.seek("mtlcolor");
    int mtlIndex = 0;
    int texIndex = -1;
    vector<string> mtlData = get<1>(settings.getCurrent());
    Material *newMaterial = new Material(mtlData);
    mtl.push_back(newMaterial);

    // Add objects/materials to list of objects/materials
    tuple<string, vector<string>> line;
    while(get<0>(line = settings.next()) != "eof"){
        string name = get<0>(line);
        if(name == "mtlcolor"){ // Set new material if new material found
            cout << "\rMaking Material";
            mtlIndex++;
            mtlData = get<1>(settings.getCurrent());
            newMaterial = new Material(mtlData);
            mtl.push_back(newMaterial);
        }
        else if(name == "texture"){
            cout << "\rMaking Texture";
            texIndex++;
            Texture *newTex = new Texture(get<1>(line));
            tex.push_back(newTex);
        }
        else if(name == "sphere"){ // Set up new sphere if sphere found
            cout << "\rMaking Sphere";
            Sphere *newSphere = new Sphere(get<1>(line), newMaterial);
            if(texIndex >= 0){
                newSphere->setTexture(tex[texIndex]);
            }
            obj.push_back(newSphere);
        }
        else if(name == "ellipsoid"){ // Set up new ellipsoid if ellipsoid found
            cout << "\rMaking Ellipsoid";
            Ellipsoid *newEllipsoid = new Ellipsoid(get<1>(line), newMaterial);
            if(texIndex >= 0){
                newEllipsoid->setTexture(tex[texIndex]);
            }
            obj.push_back(newEllipsoid);
        }
        else if(name == "v"){
            cout << "\rMaking Vertex";
            vector<string> vertData = get<1>(settings.getCurrent());
            Vertex* newVert = new Vertex(vertData);
            vert.push_back(newVert);
        }
        else if(name == "vn"){
            cout << "\rMaking Vertex Normal";
            vector<string> normData = get<1>(settings.getCurrent());
            Normal* newNorm = new Normal(normData);
            norm.push_back(newNorm);
        }
        else if(name == "vt"){
            cout << "\rMaking UV Coordinate";
            vector<string> uvData = get<1>(settings.getCurrent());
            UvCoord* newUv = new UvCoord(uvData);
            uvs.push_back(newUv);
        }
        else if(name == "f"){
            cout << "\rMaking Polygon";
            vector<string> polyData = get<1>(settings.getCurrent());
            Polygon* newPoly = new Polygon(polyData, vert, norm, uvs, mtl[mtlIndex]);
            if(texIndex >= 0){
                newPoly->setTexture(tex[texIndex]);
            }
            poly.push_back(newPoly);
            obj.push_back(newPoly);
        }
    }
    // --------------

    //Lights ----------
    cout << "\nMaking Lights\n";
    settings.seekStart();

    while(get<0>(line = settings.next()) != "eof"){
        string name = get<0>(line);
        if(name == "light"){
            cout << "\rMaking Point/Directional Light";
            vector<string> ltData = get<1>(settings.getCurrent());
            Light* newLight = new Basiclight(ltData);
            lt.push_back(newLight);
        }

        else if(name == "spotlight"){
            cout << "\rMaking Spotlight";
            vector<string> ltData = get<1>(settings.getCurrent());
            Light* newLight = new Spotlight(ltData);
            lt.push_back(newLight);
        }
    }
    // ----------------


    return 0;

}
void makeImage(ImgSettings settings){
    
    
    // Core image info
    char * token;

    token = strtok (inputFileName,".");
    string outputFileName = token;
    outputFileName = outputFileName + ".ppm";
    outputFile.open (outputFileName);
    outputFile << "P3\n";
    outputFile << "# My Image Maker\n";

    // instantiate objects
    Scene scene;
    makeObjects(settings, scene.cam, scene.img, scene.objects, scene.vertices, scene.normals, scene.uvs, scene.polygons, scene.materials, scene.textures, scene.lights);
    // Write image size to file
    outputFile << scene.img.width << " " << scene.img.height << "\n";

    // Max color value
    outputFile << scene.img.maxColor << "\n";
    
    scene.render(outputFile);
}

void readFile(char* fileName, ifstream& stream){
    cout << "Reading image info from " << fileName << "\n";
    stream.open (fileName, ifstream::in);
    if(!stream){
        cout << "File not found.\n";
        exit(EXIT_FAILURE);
    }
}

void errorExit(void){
    outputFile.close();

}