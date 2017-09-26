#include <iostream>
#include <cmath>
#include <fstream>
#include <tclap/CmdLine.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace TCLAP;

// Counter for the number of blocks in the result
int numBlocks = 0;

// Creates block at point x, y, z pointing upwards
void placeBlock( int x, int y, int z, bool plate, ofstream &output_file )
{
  output_file << "1 0 " << x*20 << " " << ( plate ? 8 : 24 ) * y << " " << z*20 << " 1 0 0 0 1 0 0 0 1 " << ( plate ? "3024.dat" : "3005.dat" ) << endl;
  ++numBlocks;
}

// Checks if the block at x, yy, z is inside the sphere or torus
bool validPoint( int x, int yy, int z, double radius, double smallRadius, bool plate )
{
  // Changes y to the correct vertical height
  double y = plate ? (double)yy * 2.0/5.0 : (double)yy * 6.0/5.0;

  // Sphere
  bool valid = ( smallRadius == 0 ) && ( x * x + y * y + z * z < radius * radius );

  // Torus
  valid = valid || ( ( smallRadius > 0 ) && ( radius - sqrt( x * x + z * z ) ) * ( radius - sqrt( x * x + z * z ) ) + y * y < smallRadius * smallRadius );

  return valid;
}

// Checks if point is valid but every block around it is not valid so that the block is not hidden behind others
bool usePoint( int x, int y, int z, double outerRadius, double radius, bool plate )
{
  return validPoint( x, y, z, outerRadius, radius, plate ) && !( 
    validPoint( x, y, z+1, outerRadius, radius, plate ) && 
    validPoint( x, y, z-1, outerRadius, radius, plate ) && 
    validPoint( x, y+1, z, outerRadius, radius, plate ) && 
    validPoint( x, y-1, z, outerRadius, radius, plate ) && 
    validPoint( x+1, y, z, outerRadius, radius, plate ) && 
    validPoint( x-1, y, z, outerRadius, radius, plate )
    );
}

// Creates a sphere or torus out of blocks facing upwards
// Sphere has radius radius
// Torus has major radius radius and minor radius smallRadius
void blockSphere( double radius, double smallRadius, bool plate, ofstream &output_file )
{
  // Compute the height of the object
  double height = ( smallRadius > 0 ) ? smallRadius : radius;
  height = plate ? 3 * height : height;

  // For every position in the bounding box
  for( int i = 0; i < smallRadius+radius; ++i )
  {
    for( int j = 0; j < height; ++j )
    {
      for( int k = 0; k < smallRadius+radius; ++k )
      {
        // If the block is within the object
        if( usePoint( i, j, k, radius, smallRadius, plate ) )
        {
          // Place the block and any valid reflection
          placeBlock( i, j, k, plate, output_file );
          if( k != 0 ) placeBlock( i, j, -k, plate, output_file );
          if( j != 0 ) placeBlock( i, -j, k, plate, output_file );
          if( j*k != 0 ) placeBlock( i, -j, -k, plate, output_file );
          if( i != 0 ) placeBlock( -i, j, k, plate, output_file );
          if( i*k != 0 ) placeBlock( -i, j, -k, plate, output_file );
          if( i*j != 0 ) placeBlock( -i, -j, k, plate, output_file );
          if( i*j*k != 0 ) placeBlock( -i, -j, -k, plate, output_file );
        }
      }
    }
  }
}

// Creates block at point x, y, z pointing away from the origin
void processPoint( double x, double y, double z, ofstream &output_file )
{
  // Create the up, forward, and right vectors for the rotation matrix
  glm::vec3 up = glm::normalize( glm::vec3( -x, -y, -z ) );
  glm::vec3 forward = glm::vec3( 0, 1, 0 );
  glm::vec3 right = glm::normalize( glm::cross( forward, up ) );
  forward = glm::normalize( glm::cross( right, up ) );

  // Write out the position and the rotation matrix
  output_file << "1 0 " << x - up[0] * 8 << " " << y - up[1] * 8 << " " << z - up[2] * 8 << " ";

  for( int i = 0; i < 3; ++i )
  {
    output_file << right[i] << " ";
    output_file << up[i] << " ";
    output_file << forward[i] << " ";
  }

  output_file << "3024.dat\n";
  ++numBlocks;
}

// Processes a single circle around the sphere
void processCircle( double radius, double y, int n, ofstream &output_file )
{
  double angle  = 2 * 3.14159265358979 / n;

  for( int i = 0; i < n; ++i )
  {
    double x = radius * cos( angle * i );
    double z = radius * sin( angle * i );

    processPoint( x, y, z, output_file );
    if( y != 0 ) processPoint( x, -y, z, output_file );
  }
}

// Creates a sphere of blocks pointing away from the origin
void outwardSphere( double radius, ofstream &output_file )
{
  // Change to lego units
  radius *= 20;

  // Get the angle between blocks and number of sides in the middle circle
  double angle = 2 * atan( 10 / radius );
  int n = int( 2 * 3.14159265358979 / angle );

  double ur, mr, y;
  int numSides;

  // Length from the origin to the furthest points on the 
  double lr = sqrt( radius * radius + 100 );

  for( int i = 0; i < n/4; ++i )
  {
    // For first level set values manually
    if( i == 0 )
    {
      y = radius * sin( angle / 2 );
      mr = sqrt( radius * radius - y * y );
      ur = lr * cos( angle);
    }
    else
    {
      // Get the intersection of the two block edges
      double a = 3.14159265358979 / numSides;
      double xx = 10 * cos(a);
      double yy = 10 * sin(a);
      double bx = (ur - yy) * yy / xx - xx;
      bx = sqrt( bx * bx + ( ur - yy ) * ( ur - yy ) );

      // Get the intersectino of the lr circle and outward edge
      bx = ( mr * sqrt( -(bx*bx-lr*lr)*y*y + 2.0*bx*sqrt(lr*lr-ur*ur)*mr*y+mr*mr*ur*ur)+y*(bx*y-sqrt(lr*lr-ur*ur)*mr) )/ (y*y+mr*mr);
      double by = sqrt(lr*lr-ur*ur);

      // Compute height of circle
      y = radius * sin( atan( by / bx ) + atan( 10 / radius ) );

      // Compute the horizontal radius at that height
      mr = sqrt( radius * radius - y * y );

      // Compute the horizontal radius at the uppermost point of the block
      ur = 2 * mr - bx;
    }

    // Compute the nomber of blocks in the circle
    numSides = int( 3.14159265358979 / atan( 10 / ur ) );

    processCircle( mr, y, numSides, output_file );
  }

  if( numSides > 3 || numSides < 0 )
  {
    output_file << "1 0 0 " << radius + 8 << " 0 -1 0 0 0 -1 0 0 0 -1 3024.dat\n";
    output_file << "1 0 0 " << -radius - 8 << " 0 1 0 0 0 1 0 0 0 1 3024.dat\n";
    numBlocks += 2;
  }
}

int main( int argc, char **argv )
{
  try
  {
    CmdLine cmd("Reads an input LDraw file corresponding to one side of a Bram/Lowell lego sphere. Converts to hollow shell made of lego pieces. Optionally maps equirectangular image to sphere using lego color pallette.", ' ', "1.0");

    ValueArg<double> smallArg( "s", "small", "Minor radius of torus", false, 0, "double");
    cmd.add( smallArg );

    ValueArg<double> radiusArg( "r", "radius", "Radius of sphere or major radius of torus", true, 1, "double");
    cmd.add( radiusArg );

    ValueArg<int> typeArg( "t", "type", "Type of object to generate", false, 1, "int");
    cmd.add( typeArg );

    ValueArg<string> outputArg( "o", "output", "Output LDraw file (.ldr)", true, "out.ldr", "string");
    cmd.add( outputArg );

    cmd.parse( argc, argv );

    string outputSphere = outputArg.getValue();
    int type            = typeArg.getValue();
    double radius       = radiusArg.getValue();
    double smallRadius  = smallArg.getValue();

    ofstream output_file( outputSphere );

    switch( type )
    {
      case 1:
        outwardSphere( radius, output_file );
        break;
      case 2:
        blockSphere( radius, 0, false, output_file );
        break;
      case 3:
        blockSphere( radius, 0, true, output_file );
        break;
      case 4:
        blockSphere( radius, smallRadius, false, output_file );
        break;
      case 5:
        blockSphere( radius, smallRadius, true, output_file );
        break;
    }

    cout << numBlocks << endl;

    output_file.close();
  }
  catch (ArgException &e)  // catch any exceptions
  {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
  }

  return 0;
}
