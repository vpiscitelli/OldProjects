#include "shader.h"
#include <vector>

shader :: shader()
{
	// Variables

	// Set a default for each and every variable
	shaderFileName = NULL;

	ShaderType = 0;
	shaderNumber = 0;
}


shader :: ~shader()
{
	// Variables

	// Delete shaderFileName
	delete []shaderFileName;

	// Set to nUll
	shaderFileName = NULL;
}



void shader :: setShaderFileName( const char * FileName)
{
	// Variables
	int slFileName = strlen(FileName); // Get the size of the file name

	// Allocate memory after setting it to null originally
	shaderFileName = new char[slFileName +1];

	// Strcpy it over
	strcpy(shaderFileName, FileName);

	// End the termination with null
	shaderFileName[slFileName] = '\0';
}

// Read into the content variable
bool shader :: readIn(void)
{
	// Variable
	ifstream source(shaderFileName);
	ifstream output; // Just in case for debugging purposes

	if(source.good())
	{
		string line;

		while(getline(source, line))
		{
			content.append(line);
			content.append("\n"); // Null terminated;	
		}
	
		// Close file
		source.close();
		return true;
	}	
	return false;
}

// ShaderLoader/Compiler
GLuint shader :: shaderLoader(GLenum shaderType)
{
	// Variables
	GLuint shaderID = 0;

	// Set shader Type for class
	ShaderType = shaderType;

	shaderID = glCreateShader(shaderType);

	const GLchar* Source = content.c_str();
	glShaderSource(shaderID, 1, (const GLchar**)&Source, NULL);

	// Compile shader Source
	glCompileShader(shaderID);

	// Check for bad compilation
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);

	if(shaderCompiled != GL_TRUE)
	{	
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

		for(int i = 0; i < errorLog.size(); i++)
		{
			cout << errorLog[i];
		}

		cout << endl << "shader file: " << shaderFileName << " failed to compile" << endl;
		shaderID = 0;
	}

	// Set shaderNumber/ ID
	shaderNumber = shaderID;
	return shaderID;
}
