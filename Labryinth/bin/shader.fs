const int max =1;

// Struct for distant lighting, 
struct DistantLight
{
   //vec3 Color; // Color will not be included because the colors for the textures is hard to get
   vec3 vDirection; // The direction in terms of location
   vec3 ambient; // The normal lighting for it, Could be removed in terms of normal ambient lighting as opposed to a seperate ambient lighting for this specific type of lighting
   vec3 diffuse; // The type of diffuse
   vec3 specular; // The bouncing off material

};

// Same as above but for spot light
struct SpotLight
{
    vec3 position; // Has a position, in which it's location above the object or such
    vec3 direction; // The direction, can be pointed left or right, or middle or down

    // Same as above
    vec3 ambient; 
    vec3 diffuse;
    vec3 specular; 

    // The inner vs the outer cut off point, to calculate how big the circle is
    float innerCut;
    float spotCosOuterCutOff;

    // Values to calculate attenuation
    float constant;
    float linear;
    float quadratic;     
};

struct PointLight
{   vec3 position;

   vec4 ambient;
   vec4 diffuse;
   vec4 specular; 

   float constant;
   float linear;
   float quadratic;

  
};

// Uniform variables
uniform DistantLight directionalLight; // This is a directional Light variable which will receive its info from src
uniform SpotLight spotLight[2]; // Create an array of spotLights, each of which will have all those data within from the src file
uniform sampler2D gSampler; // Sampler to calculate the texture
uniform PointLight pLight[1]; // Create an array of pointLights, each of which will store the data
uniform vec4 ambientLight; // Ambient data that is applied to every pixel

// Varying variables
varying vec2 texture; // Texture coordinates from the vertex shader that has x and y coordinates
varying vec3 N; // Normal from the shader file
varying vec3 l_Position; // v_position of the models
varying vec3 model_fragPos; // Position of the model in vec3 format
varying vec3 vv; // E vector

void main( void )
   {
      {
        // This is for directional lighting only

        // Normalize the directional Light in terms of the
        vec3 L = normalize(-directionalLight.vDirection );

        // This takes the direction of the light, and direction of the normal
        vec3 R = normalize(-reflect(L, N));

        // The intensity of the light multiplied by the dot product of the max out vector after calculating normals

        vec3 diffuse = directionalLight.diffuse ;//* max(dot(N, L), 0.0);

        // The same thing as above, however, this is the amount of shiny lights that are passed back, 50.0 how shiny we want the object to be or is the object, while the other thing calculates it
        vec3 specular = directionalLight.specular ;

       // Add it to the vec4 
       gl_FragColor = vec4((specular + diffuse + directionalLight.ambient), 1.0);  
      }

      {
          // Loop through the two lights
          for(int i = 0; i < 1; i++)
            {
              // Do the same thing as above in terms of calculations, however, keep in mind, that the light goes to the object itself. So essnetially, model_fragPos is the object that is passed from the main
              vec3 L = normalize(spotLight[i].position- model_fragPos);
              vec3 E = normalize(-model_fragPos);
              vec3 R = normalize(-reflect(L, N));

              vec3 LD = normalize(spotLight[i].position - vv);
              vec3 D = normalize(spotLight[i].direction);

              // Calculate the distance from the current light source to the object, so for each object
              float distance = length(spotLight[i].position - model_fragPos);
              
              // Calculate attenuation based on how far the light object gets from the object itself
              float attenuation =  1.0f / (spotLight[i].constant + spotLight[i].linear * distance + spotLight[i].quadratic * (distance * distance));   


              // The intensity of the light based on the dot product the opposite normalized direction of the light with the position of the light relative to the object, in which divides the light so that it // makes a circle, and clamps the value so that the value is at 0.0, to show that the spotlight exist and does not get inverted, and 1.0, so it does not spread throughout the whole scene.
              float intensity = clamp((dot(L, -D) - spotLight[i].spotCosOuterCutOff) / (spotLight[i].innerCut -  spotLight[i].spotCosOuterCutOff ), 0.0, 1.0);

              // Diffuse and specular lighting coefficient based on the direction and reflection, very much like distant
              float kd = 0.5;//max(dot(N, spotLight[i].direction), 0.0);
              float ks = 0.5;// pow(max(dot(R, E), 0.0), 10.0f);

              vec3 ambientLight = spotLight[i].ambient ;
              vec3 diffuseLight = spotLight[i].diffuse * kd ;
              vec3 specularLight = spotLight[i].specular * ks;

              // Multiply, do not add
              ambientLight  *= attenuation * intensity;
              diffuseLight *= attenuation * intensity;
              specularLight *= attenuation * intensity;

              gl_FragColor += vec4((ambientLight + diffuseLight + specularLight).rgb, 1.0);
            }
      } 

      {
        // Only one point light so do the same thing as above also can calulate the circle itself just like above, but didn't want to.
      for(int i = 0; i < 1 ; i++)
          {
            vec3 L = normalize(pLight[i].position - model_fragPos);
            vec3 E = normalize(-model_fragPos);
            vec3 R = normalize(-reflect(L, N));


            float kd = 0.5f;//max(dot(N, E), 0.0);

            vec3 reflectDirection = reflect(-E, N);
            float ks = 0.7f;//pow(max(dot(l_Position, reflectDirection), 0.0), 40.0f);

            vec4 pambient = pLight[i].ambient;
            vec4 pdiffuse = pLight[i].diffuse * kd ;
            vec4 pspecular = pLight[i].specular  * ks;

            gl_FragColor *= (pambient + pdiffuse + pspecular);
            
          }
      }

    gl_FragColor *= ambientLight;

   gl_FragColor *= texture2D( gSampler, texture.xy );
   }