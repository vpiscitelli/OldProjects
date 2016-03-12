/*
Zeeshan Sajid's Shader Header File

This file contains the declaration of variables and functions for the fragment and vertext shader 
*/


#ifndef SHADER_H
#define SHADER_H

// Header Files
#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
using namespace std; 


class shader
	{
	 public:
		// Constructor and Destructor
		shader();
		~shader();

		// Set the file name to read the shader Info from
		void setShaderFileName( const char * FileName);
		

		// Read into the content variable
		bool readIn(void);

		// ShaderLoader/Compiler
		GLuint shaderLoader(GLenum shaderType);
		


	 private:
		char *shaderFileName;
		string content;
		GLenum ShaderType;
		GLuint shaderNumber;
	};
#endif 
