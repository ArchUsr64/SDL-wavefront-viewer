#include <iostream>
#include <math.h>
#include <SDL2/SDL.h>
#include <fstream>
#include <string>

// clang-format off
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PI 3.141592653589
#define Radians(x) x *PI / 180
#define RotationArrayAboutX(x) {{1, 0, 0},{0, cos(x), sin(x)},{0, -sin(x), cos(x)}}
#define RotationArrayAboutY(x) {{cos(x), 0, -sin(x)},{0, 1, 0},{sin(x), 0, cos(x)}}
#define RotationArrayAboutZ(x) {{cos(x), sin(x), 0},{-sin(x), cos(x), 0},{0, 0, 1}}
// clang-format on

std::string FILE_NAME = "untitled.obj"; //enter the file name for the wavefront here with the extension

float fov = 120; //field of view of the camera in degree

class model
{
public:
  float *coordinate_pointer;
  int *face_pointer;
  int totalVertices;
  int totalFaces;
  model(int p_totalVertices, int p_toatalFaces, float *p_pointerToTheCoordinateArray, int *p_pointerToTheFaceArray)
  {
    totalVertices = p_totalVertices;
    totalFaces = p_toatalFaces;
    coordinate_pointer = p_pointerToTheCoordinateArray;
    face_pointer = p_pointerToTheFaceArray;
  }
  float getCoordinate(int p_coordinateIndex, int p_whichValue) { return *(coordinate_pointer + p_coordinateIndex * 3 + p_whichValue); }
  float getFace(int p_faceIndex, int p_whichCoordinate) { return *(face_pointer + p_faceIndex * 3 + p_whichCoordinate); }
};

class matrix //matrix class, only used indirectly as a middle step in matrix computations, arrays used everywhere else
{
public:
  int rows, columns;
  float *elements;
  matrix(int p_rows, int p_columns, float *p_arrayPointer)
  {
    rows = p_rows;
    columns = p_columns;
    elements = p_arrayPointer;
  }
  float getElement(int p_row, int p_column) //method returns value at (p_row, p_column) inside the matrix
  {
    if (p_row > rows | p_column > columns)
      std::cout << "No value at the index specified: OUT OF BOUND OF THE MATRIX" << std::endl;
    return *(elements + columns * p_row + p_column);
  }
  void storeElement(int p_row, int p_column, float p_value) //method used to store p_value in the matrix
  {
    if (p_row > rows | p_column > columns)
      std::cout << "No value at the index specified: OUT OF BOUND OF THE MATRIX" << std::endl;
    *(elements + columns * p_row + p_column) = p_value;
  }
};

matrix matrixMultiplication(matrix p_m1, matrix p_m2, float *p_pointer) //function to multiply p_m1 and p_m2 and returns the product with elements stored at p_pointer
{
  float resultantArray[p_m1.rows][p_m2.columns];
  for (int i = 0; i < p_m1.rows; i++)
  {
    for (int j = 0; j < p_m2.columns; j++)
    {
      resultantArray[i][j] = 0;
    }
  }
  if (p_m1.columns != p_m2.rows)
  {
    std::cout << "ERROR: matrix multiplication error" << std::endl;
  }
  else
  {
    for (int i = 0; i < p_m1.rows; i++)
    {
      for (int j = 0; j < p_m2.columns; j++)
      {
        for (int k = 0; k < p_m2.rows; k++)
        {
          resultantArray[i][j] += p_m1.getElement(i, k) * p_m2.getElement(k, j);
        }
      }
    }
  }
  for (int i = 0; i < p_m1.rows; i++)
  {
    for (int j = 0; j < p_m2.columns; j++)
    {
      *(p_pointer + i * p_m2.columns + j) = resultantArray[i][j];
    }
  }
  matrix resultantMatrix = matrix(p_m1.rows, p_m2.columns, p_pointer);
  return resultantMatrix;
}

matrix matrixAddition(matrix p_m1, matrix p_m2, float *p_pointer) //function to add p_m1 and p_m2 and return the result with elements stored at p_pointer
{
  float resultantArray[p_m1.rows][p_m1.columns];
  for (int i = 0; i < p_m1.rows; i++)
  {
    for (int j = 0; j < p_m2.columns; j++)
    {
      resultantArray[i][j] = 0;
    }
  }
  if (p_m1.rows != p_m2.rows | p_m1.columns != p_m2.columns)
  {
    std::cout << "ERROR: matrix addition error" << std::endl;
  }
  else
  {
    for (int i = 0; i < p_m1.rows; i++)
    {
      for (int j = 0; j < p_m1.columns; j++)
      {
        resultantArray[i][j] = p_m1.getElement(i, j) + p_m2.getElement(i, j);
      }
    }
  }
  for (int i = 0; i < p_m1.rows; i++)
  {
    for (int j = 0; j < p_m1.columns; j++)
    {
      *(p_pointer + i * p_m1.columns + j) = resultantArray[i][j];
    }
  }
  matrix resultantMatrix = matrix(p_m1.rows, p_m1.columns, p_pointer);
  return resultantMatrix;
}

void drawTriangle(SDL_Renderer *p_renderer, float p_c1[], float p_c2[], float p_c3[]) //draws a triangle with coordinates passed as 2 sized float arrays
{
  SDL_RenderDrawLine(p_renderer, p_c1[0], p_c1[1], p_c2[0], p_c2[1]);
  SDL_RenderDrawLine(p_renderer, p_c1[0], p_c1[1], p_c3[0], p_c3[1]);
  SDL_RenderDrawLine(p_renderer, p_c2[0], p_c2[1], p_c3[0], p_c3[1]);
}

void copySelectedRotationArrayTypeToThePointerProvided(int p_mode, float *p_pointer, float p_angle) //copies the 3x3 rotation transformation matrix to p_pointer with p_mode defining the axis to rotate about
{
  if (p_mode == 0)
  {
    float dummyArray[3][3] = RotationArrayAboutX(p_angle);
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        *(p_pointer + i * 3 + j) = dummyArray[i][j];
      }
    }
  }
  if (p_mode == 1)
  {
    float dummyArray[3][3] = RotationArrayAboutY(p_angle);
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        *(p_pointer + i * 3 + j) = dummyArray[i][j];
      }
    }
  }
  if (p_mode == 2)
  {
    float dummyArray[3][3] = RotationArrayAboutZ(p_angle);
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        *(p_pointer + i * 3 + j) = dummyArray[i][j];
      }
    }
  }
}

int verticesInModel(std::string p_fileName)
{
  std::ifstream inFile;
  inFile.open(p_fileName);
  int totalNumberOfVertices = 0;

  if (inFile.fail())
  {
    std::cout << "Error opening " << p_fileName << std::endl;
    return -1;
  }

  std::string item;

  while (std::getline(inFile, item))
  {
    if (item[0] == 'v')
      totalNumberOfVertices++;
  }
  return totalNumberOfVertices;
}

int facesInModel(std::string p_fileName)
{
  std::ifstream inFile;
  inFile.open(p_fileName);
  int totalNumberOfFaces = 0;

  if (inFile.fail())
  {
    std::cout << "Error opening " << p_fileName << std::endl;
    return -1;
  }

  std::string item;

  while (std::getline(inFile, item))
  {
    if (item[0] == 'f')
      totalNumberOfFaces++;
  }
  return totalNumberOfFaces;
}

float floatify(std::string p_input)
{
  int constituentNumbers[p_input.length()];
  bool hasDecimal = 0;
  bool isNegative = 0;
  int numberOfCharactersToIgnore = 0;
  for (int i = 0; i < p_input.length(); i++)
  {
    constituentNumbers[i] = -3;
    switch (p_input[i])
    {
    case '1':
      constituentNumbers[i] = 1;
      break;
    case '2':
      constituentNumbers[i] = 2;
      break;
    case '3':
      constituentNumbers[i] = 3;
      break;
    case '4':
      constituentNumbers[i] = 4;
      break;
    case '5':
      constituentNumbers[i] = 5;
      break;
    case '6':
      constituentNumbers[i] = 6;
      break;
    case '7':
      constituentNumbers[i] = 7;
      break;
    case '8':
      constituentNumbers[i] = 8;
      break;
    case '9':
      constituentNumbers[i] = 9;
      break;
    case '0':
      constituentNumbers[i] = 0;
      break;
    case '-':
      constituentNumbers[i] = -2;
      isNegative = 1;
      break;
    case '.':
      constituentNumbers[i] = -1;
      hasDecimal = 1;
      break;
    default:
      numberOfCharactersToIgnore++;
    }
  }
  int finalConstituentNumbers[p_input.length() - numberOfCharactersToIgnore - isNegative];

  int i = 0;
  for (int j = 0; j < p_input.length(); j++)
  {
    if (constituentNumbers[j] >= -1)
    {
      finalConstituentNumbers[i] = constituentNumbers[j];
      i++;
    }
  }
  int totalDigits = sizeof(finalConstituentNumbers) / sizeof(int);
  int digitsAfterDecimal = totalDigits;
  for (int i = 0; i < totalDigits; i++)
  {
    if (finalConstituentNumbers[i] == -1)
    {
      digitsAfterDecimal = i + 1;
      break;
    }
  }
  digitsAfterDecimal = totalDigits - digitsAfterDecimal;

  float finalResult = 0;
  int placeOfCurrentDigit = 0;
  for (int i = totalDigits - 1; i >= 0; i--)
  {
    if (finalConstituentNumbers[i] != -1)
    {
      finalResult += finalConstituentNumbers[i] * pow(10, placeOfCurrentDigit);
      placeOfCurrentDigit++;
    }
  }
  finalResult *= pow(10, -digitsAfterDecimal);
  if (isNegative)
    finalResult *= -1;
  return finalResult;
}

model loadModel(std::string p_fileName, float *p_coordinateArrayPointer, int *p_facesArrayPointer)
{
  model returnModel = model(verticesInModel(p_fileName), facesInModel(p_fileName), p_coordinateArrayPointer, p_facesArrayPointer);
  std::ifstream inFile;
  inFile.open(p_fileName);
  if (inFile.fail())
  {
    std::cout << "Error opening " << p_fileName << std::endl;
    return returnModel;
  }

  std::string lineItem;
  int coordArrayIndex = 0;
  int faceArrayIndex = 0;
  while (std::getline(inFile, lineItem))
  {
    std::string value;
    int numStart[3];
    int numEnd[3];
    if (lineItem[0] == 'v')
    {
      lineItem.replace(0, 1, " ");
      int spaceCharacterPermutationsFound = 0;
      for (int i = 1; i < lineItem.length(); i++)
      {
        if (lineItem[i] != ' ' & lineItem[i - 1] == ' ' & spaceCharacterPermutationsFound % 2 == 0)
        {
          numStart[spaceCharacterPermutationsFound / 2] = i;
          spaceCharacterPermutationsFound++;
        }
        if (lineItem[i] != ' ' & (lineItem[i + 1] == ' ' | i == lineItem.length() - 1) & spaceCharacterPermutationsFound % 2 == 1)
        {
          numEnd[spaceCharacterPermutationsFound / 2] = i;
          spaceCharacterPermutationsFound++;
        }
      }
      for (int i = 0; i < 3; i++)
      {
        value = lineItem.substr(numStart[i], numEnd[i] - numStart[i] + 1);
        *(p_coordinateArrayPointer + i + coordArrayIndex * 3) = floatify(value);
      }
      coordArrayIndex++;
    }
    if (lineItem[0] == 'f')
    {
      lineItem.replace(0, 1, " ");
      int spaceCharacterPermutationsFound = 0;
      for (int i = 1; i < lineItem.length(); i++)
      {
        if (lineItem[i] != ' ' & lineItem[i - 1] == ' ' & spaceCharacterPermutationsFound % 2 == 0)
        {
          numStart[spaceCharacterPermutationsFound / 2] = i;
          spaceCharacterPermutationsFound++;
        }
        if (lineItem[i] != ' ' & (lineItem[i + 1] == ' ' | i == lineItem.length() - 1) & spaceCharacterPermutationsFound % 2 == 1)
        {
          numEnd[spaceCharacterPermutationsFound / 2] = i;
          spaceCharacterPermutationsFound++;
        }
      }
      for (int i = 0; i < 3; i++)
      {
        value = lineItem.substr(numStart[i], numEnd[i] - numStart[i] + 1);
        *(p_facesArrayPointer + i + faceArrayIndex*3) = floatify(value);
      }
      faceArrayIndex++;
    }
  }
  return returnModel;
}

int main()
{
  float coords[verticesInModel(FILE_NAME)][3];
  int faces[facesInModel(FILE_NAME)][3];
  model susane = loadModel(FILE_NAME, &coords[0][0], &faces[0][0]);

#pragma region //SDL initial setup

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
  {
    std::cout << "ERROR INITIALISING SDL: " << SDL_GetError() << std::endl;
    return 1;
  }

  //creates a window
  SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!win)
  {
    std::cout << "ERROR CREATING WINDOW: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  //creates a renderer
  Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  SDL_Renderer *rend = SDL_CreateRenderer(win, -1, render_flags);
  if (!rend)
  {
    std::cout << "ERROR CREATING RENDERER: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

#pragma endregion

  fov = Radians(fov);                                                                                                                //converts fov from degree to radians
  float final2DCoords[susane.totalVertices][3];                                                                                      //array that stores final 2D perspectively projected coordinates from "coords" array
  bool programRunning = true;                                                                                                        //boolean to detect if user wants to end the application
  SDL_Event ev;                                                                                                                      //initialise an SDL_Event union
  float translationArray[3][1] = {{0}, {0}, {-3}};                                                                                   //array to store translation matrix
  float rotationArray[3][3];                                                                                                         //array to store rotation matrix
  float scalar = 1;                                                                                                                  //the factor by which to scale the mesh
  float rotationAngle = 0;                                                                                                           //float to store the rotation of the mesh in radians
  bool key_up = 0, key_left = 0, key_down = 0, key_right = 0, key_inside = 0, key_outside = 0, key_sizeGrow = 0, key_sizeShrink = 0; //all keys for input
  int timesTabPressed = 1;

  while (programRunning) //main window refresh loop
  {
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255); //set background color to black
    SDL_RenderClear(rend);

    copySelectedRotationArrayTypeToThePointerProvided(timesTabPressed % 3, &rotationArray[0][0], rotationAngle); //set the values for "rotationArray"
    matrix rotationMatrix = matrix(3, 3, &rotationArray[0][0]);

    matrix translationMatrix = matrix(3, 1, &translationArray[0][0]); //create a matrix from "translationArray"

    while (SDL_PollEvent(&ev)) //gets the key states as well as handling quit input
    {
      switch (ev.key.type)
      {
      case SDL_QUIT:
        programRunning = false;
        break;
      case SDL_KEYDOWN:
        switch (ev.key.keysym.scancode)
        {
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:
          key_up = 1;
          break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:
          key_left = 1;
          break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
          key_down = 1;
          break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT:
          key_right = 1;
          break;
        case SDL_SCANCODE_SPACE:
          key_inside = 1;
          break;
        case SDL_SCANCODE_LSHIFT:
          key_outside = 1;
          break;
        case SDL_SCANCODE_PAGEUP:
          key_sizeGrow = 1;
          break;
        case SDL_SCANCODE_PAGEDOWN:
          key_sizeShrink = 1;
          break;
        case SDL_SCANCODE_TAB:
          timesTabPressed++;
          break;
        }
        break;
      case SDL_KEYUP:
        switch (ev.key.keysym.scancode)
        {
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:
          key_up = 0;
          break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:
          key_left = 0;
          break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
          key_down = 0;
          break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT:
          key_right = 0;
          break;
        case SDL_SCANCODE_SPACE:
          key_inside = 0;
          break;
        case SDL_SCANCODE_LSHIFT:
          key_outside = 0;
          break;
        case SDL_SCANCODE_PAGEUP:
          key_sizeGrow = 0;
          break;
        case SDL_SCANCODE_PAGEDOWN:
          key_sizeShrink = 0;
          break;
        }
        break;
      }
    }

    scalar += (key_sizeGrow - key_sizeShrink) * 0.05;                                                             //change the scalar value based on key input
    translationMatrix.storeElement(0, 0, translationMatrix.getElement(0, 0) + (key_right - key_left) * 0.05);     //change the amout of translation in x direction
    translationMatrix.storeElement(1, 0, translationMatrix.getElement(1, 0) + (key_up - key_down) * 0.05);        //change the amout of translation in y direction
    translationMatrix.storeElement(2, 0, translationMatrix.getElement(2, 0) + (key_outside - key_inside) * 0.05); //change the amout of translation in z direction

    for (int i = 0; i < susane.totalVertices; i++) //handles scaling of the mesh
    {
      for (int j = 0; j < 3; j++)
      {
        final2DCoords[i][j] = scalar * coords[i][j];
      }
    }

    for (int i = 0; i < susane.totalVertices; i++) //rotation + translation of the mesh
    {
      float currentCoordsArray[3][1] = {{final2DCoords[i][0]}, {final2DCoords[i][1]}, {final2DCoords[i][2]}}; //temporary variable for computation purposes
      matrix currentCoords = matrix(3, 1, &currentCoordsArray[0][0]);

      float dummyArray_2[rotationMatrix.rows][currentCoords.columns]; //applies the rotation transformation
      currentCoords = matrixMultiplication(rotationMatrix, currentCoords, &dummyArray_2[0][0]);

      float dummyArray_3[currentCoords.rows][currentCoords.columns]; //applies translation
      currentCoords = matrixAddition(currentCoords, translationMatrix, &dummyArray_3[0][0]);

      for (int j = 0; j < 3; j++)
      {
        final2DCoords[i][j] = currentCoords.getElement(j, 0); //stores the temporary variable value to "final2DCoords"
      }
    }

    for (int i = 0; i < susane.totalVertices; i++) //handles transformation of 3D points to the SDL standard 2D screen coordinate system
    {
      for (int j = 0; j < 2; j++) //perespective projection from 3D space onto 2D space
      {
        final2DCoords[i][j] = final2DCoords[i][j] / (1 - final2DCoords[i][2] * tan(fov / 2));
      }

      final2DCoords[i][0] = (final2DCoords[i][0] + 1) * WINDOW_WIDTH / 2;                                                //transforming the x coordinate from 0-1 space to the screen
      final2DCoords[i][1] = (((final2DCoords[i][1] * (-1.0 * WINDOW_WIDTH)) / WINDOW_HEIGHT) + 1) * WINDOW_HEIGHT / 2.0; //transforming the y coordinate from 0-1 space to the screen
    }

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255); //sets draw colour to white for drawing the triangles

    for (int i = 0; i < susane.totalFaces; i++)
    {
      if (final2DCoords[faces[i][0]- 1][2] < 0 & final2DCoords[faces[i][1]- 1][2] < 0 & final2DCoords[faces[i][2]- 1][2] < 0) //check if the face is behind the screen
        drawTriangle(rend, final2DCoords[faces[i][0] - 1], final2DCoords[faces[i][1]- 1], final2DCoords[faces[i][2]- 1]);
    }
    // rotationAngle += 0.01;
    SDL_RenderPresent(rend);
  }

  SDL_DestroyRenderer(rend); //closing SDL
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 1;
}
