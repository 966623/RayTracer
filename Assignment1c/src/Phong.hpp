#ifndef Phong_HPP
#define Phong_HPP
#define _USE_MATH_DEFINES
#include <tuple>
#include <vector>
#include <sstream>
#include <string.h>
#include <math.h>
#include <iostream>
#include "Vec3.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "Light.hpp"
#include "Object.hpp"
#include "Camera.hpp"
#include "Polygon.hpp"

using namespace std;

class Phong {
	public:
		Phong(){

		}

		Vec3 calc(Vec3 &intersect, Camera *c, Object *o, vector<Light*> &l, vector<Object*> &obj){
			Material* mat = o->material;
			Texture* tex = o->texture;
			Vec3 color;
			color = o->getColor(intersect);
			//if(o->textured){
			//	color = tex->getColor(intersect, ((Polygon*)o)->uvs, ((Polygon*)o)->vertices);
			//}
			//else{
			//	color = mat->color;
			//}

			Vec3 N = o->getNormal(intersect);
			Vec3 V = (c->pos - intersect).normalized();

			color = (mat->ka * color); //Ambient
			for(int i = 0; i < l.size(); i++){
				int S = 1;
				Light* currentLight = l[i];
				Vec3 L = currentLight->getDir(intersect);
	            float distance = -1;
	            Object* closestObject = obj[0];
				//If light touches point
				if(L.magnitude() > 0){
					//Check shadow
		            // Check intersection with every object
		            for(int i = 0; i < obj.size(); i++){
		            	if(obj[i] != o){
		            		float dist = obj[i]->getIntersect(intersect, L);

		            		//Only surfaces facing towards the point will cast a shadow
		            		//Avoid terminator problem in smooth shading
		            		Vec3 p = intersect + L * dist;
		            		Vec3 n = obj[i]->getNormal(p);
		            		Vec3 negativeL = L * -1;
		            		if(n.dot(negativeL) >= 0){
		            			if(distance < 0 || (dist < distance && dist >= 0)){
		            				if(obj[i] != o){
		            					distance = dist;
				                    	closestObject = obj[i];
		            				}
				                
				                }
		            		}

			                
		            	}
		                
		            }
				}
				

	            if(distance <= 0.0001){
	            	//No shadow
	            	S = 1;
	            }
	            else{
	            	//If global light (non point)
	            	if(currentLight->isGlobal){
	            		S = 0;
	            	}
	            	else{
	            		if(distance <= (currentLight->pos-intersect).magnitude()){
	            			S = 0;
	            		}
	            		else{
	            			S=1;
	            		}
	            	}
	            }

				Vec3 H = (L + V).normalized();
				Vec3 currentColor = (mat->kd * color * N.dot(L)) + (mat->ks * mat->specColor * pow(N.dot(H), mat->n));
				currentColor.clamp(0,1);
				color = color + S * Vec3(currentLight->color.x * currentColor.x, currentLight->color.y * currentColor.y, currentLight->color.z * currentColor.z);
				
			}
			color.clamp(0,1);
			return color;
		}
		

		

	private:
		
}; // end SomeClass

#endif
