#include "SphereGenerator.h"

// Creates block at point x, y, z pointing upwards
void placeBlock( double x, double y, double z, bool plate, ofstream &outputFile )
{
  outputFile << "1 0 " << x*20 << " " << ( plate ? 8 : 24 ) * y << " " << z*20 << " 1 0 0 0 1 0 0 0 1 " << ( plate ? "3024.dat" : "3005.dat" ) << endl;
}

// Creates block at point x, y, z pointing upwards
void placeBlock2( double x, double y, double z, bool type, ofstream &outputFile )
{
  outputFile << "1 0 " << x*20 << " " << 8 * y << " " << z*20 << " 0 0 1 0 1 0 -1 0 0 " << (type ? "3070.dat" : "3024.dat") << endl;
  outputFile << "1 0 " << x*20 << " " << -8 * y << " " << z*20 << " 0 0 -1 0 -1 0 -1 0 0 " << (type ? "3070.dat" : "3024.dat") << endl;
  outputFile << "1 0 " << 8 * y << " " << 20 * z << " " << 20 * x << " 0 1 0 -1 0 0 0 0 1 " << (type ? "3070.dat" : "3024.dat") << endl;
  outputFile << "1 0 " << -8 * y << " " << 20 * z << " " << 20 * x << " 0 -1 0 1 0 0 0 0 1 " << (type ? "3070.dat" : "3024.dat") << endl;
  outputFile << "1 0 " << z*20 << " " << x*20 << " " << 8 * y << " 1 0 0 0 0 -1 0 1 0 " << (type ? "3070.dat" : "3024.dat") << endl;
  outputFile << "1 0 " << 20 * z << " " << 20 * x << " " << -8 * y << " 1 0 0 0 0 1 0 -1 0 " << (type ? "3070.dat" : "3024.dat") << endl;
}

// Checks if the block at x, yy, z is inside the sphere
bool validPoint( double x, double yy, double z, double radius, bool plate )
{
  // Changes y to the correct vertical height
  double y = plate ? (double)yy * 2.0/5.0 : (double)yy * 6.0/5.0;

  // Sphere
  bool valid = ( x * x + y * y + z * z < radius * radius );

  return valid;
}

// Checks if point is valid but every block around it is not valid so that the block is not hidden behind others
bool usePoint( double x, double y, double z, double radius, bool plate )
{
  return validPoint( x, y, z, radius, plate ) && !( 
    validPoint( x, y, z+1, radius, plate ) && 
    validPoint( x, y, z-1, radius, plate ) && 
    validPoint( x, y+1, z, radius, plate ) && 
    validPoint( x, y-1, z, radius, plate ) && 
    validPoint( x+1, y, z, radius, plate ) && 
    validPoint( x-1, y, z, radius, plate )
    );
}

bool usePoint2( double x, double y, double z, double radius, bool plate )
{
  return validPoint( x, y, z, radius, plate ) && !( 
    validPoint( x, y, z+1, radius, plate ) && 
    validPoint( x, y+1, z, radius, plate ) && 
    validPoint( x, y-1, z, radius, plate ) && 
    validPoint( x+1, y, z, radius, plate ) && 
    validPoint( x-1, y, z, radius, plate )
    );
}

// Creates a sphere out of blocks stacked upwards
void blockSphere( double radius, bool plate, ofstream &outputFile )
{
  // Compute the height of the object
  double height = radius;
  height = plate ? 3 * height : height;

  // For every position in the bounding box
  for( int i = 0; i < radius; ++i )
  {
    for( int j = 0; j < height; ++j )
    {
      for( int k = 0; k < radius; ++k )
      {
        // If the block is within the object
        if( usePoint( i, j, k, radius, plate ) )
        {
          // Place the block and any valid reflection
          placeBlock( i, j, k, plate, outputFile );
          if( k != 0 ) placeBlock( i, j, -k, plate, outputFile );
          if( j != 0 ) placeBlock( i, -j, k, plate, outputFile );
          if( j*k != 0 ) placeBlock( i, -j, -k, plate, outputFile );
          if( i != 0 ) placeBlock( -i, j, k, plate, outputFile );
          if( i*k != 0 ) placeBlock( -i, j, -k, plate, outputFile );
          if( i*j != 0 ) placeBlock( -i, -j, k, plate, outputFile );
          if( i*j*k != 0 ) placeBlock( -i, -j, -k, plate, outputFile );
        }
      }
    }
  }
}

// Creates block at point x, y, z pointing away from the origin
void processPoint( double x, double y, double z, ofstream &outputFile )
{
  // Create the up, forward, and right vectors for the rotation matrix
  glm::vec3 up = glm::normalize( glm::vec3( -x, -y, -z ) );
  glm::vec3 forward = glm::vec3( 0, 1, 0 );
  glm::vec3 right = glm::normalize( glm::cross( forward, up ) );
  forward = glm::normalize( glm::cross( right, up ) );

  // Write out the position and the rotation matrix
  outputFile << "1 0 " << x - up[0] * 8 << " " << y - up[1] * 8 << " " << z - up[2] * 8 << " ";

  for( int i = 0; i < 3; ++i )
  {
    outputFile << right[i] << " ";
    outputFile << up[i] << " ";
    outputFile << forward[i] << " ";
  }

  outputFile << "3024.dat\n";
}

// Processes a single circle around the sphere
void processCircle( double radius, double y, int n, ofstream &outputFile )
{
  double angle  = 2 * 3.14159265358979 / n;

  for( int i = 0; i < n; ++i )
  {
    double x = radius * cos( angle * i );
    double z = radius * sin( angle * i );

    processPoint( x, y, z, outputFile );
    if( y != 0 ) processPoint( x, -y, z, outputFile );
  }
}

// Creates a sphere of blocks pointing away from the origin
void outwardSphere( double radius, ofstream &outputFile )
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

    processCircle( mr, y, numSides, outputFile );
  }

  if( numSides > 3 || numSides < 0 )
  {
    outputFile << "1 0 0 " << radius + 8 << " 0 -1 0 0 0 -1 0 0 0 -1 3024.dat\n";
    outputFile << "1 0 0 " << -radius - 8 << " 0 1 0 0 0 1 0 0 0 1 3024.dat\n";
  }
}

void lowellSphere( double radius, ofstream &outputFile )
{
  // Compute the height of the object
  double height = radius;
  height = 2.5 * height;

  double cubeHeight = radius / sqrt(3.0);
  cubeHeight = floor(2.0*cubeHeight)/2.0;

  double maxHeight = -height; 
  double minMiddleHeight = -height; 

  // For every position in the bounding box
  for( double i = -cubeHeight+0.5; i < cubeHeight+0.5; ++i )
  {
    for( double j = -2.5*cubeHeight-1; j > -height; --j )
    {
      for( double k = -cubeHeight+0.5; k < cubeHeight+0.5; ++k )
      {
        // If the block is within the object
        if( usePoint( i, j, k, radius, true ) || ( validPoint( i, j, k, radius, true ) && ( i == -cubeHeight+0.5 || i == cubeHeight - 0.5) ) )
        {
          // Place the block and any valid reflection
          if( ( (i == -cubeHeight + 0.5 || i == cubeHeight - 0.5) && j <= minMiddleHeight ) || (usePoint( i, j, k, radius, true ) && !( k == -cubeHeight+0.5 && usePoint( i, j, k - 0.75, radius, true ) ) && !( k == cubeHeight - 0.5 && usePoint( i, j, k + 0.75, radius, true ) ) ) )
          {
            placeBlock2( i, j, k, false, outputFile );

            minMiddleHeight = max( minMiddleHeight, j );
          }

          maxHeight = max( maxHeight, j );
        }
      }
    }
  }

  // For every position in the bounding box
  for( double i = -cubeHeight+0.5; i < cubeHeight+0.5; ++i )
  {
    for( double j = maxHeight; j > -height; --j )
    {
      for( double k = -round(2.0*radius)/2.0; k < -cubeHeight + 0.5; k += 0.5 )
      {
        // If the block is within the object
        if( usePoint( i, j, k-0.25, radius, true ) )
        {
          // Place the block and any valid reflection
          if( ( usePoint( i, j-1, k-0.25, radius, true ) || !usePoint( i, j-1, k + 0.25, radius, true ) ) && k < -cubeHeight )
          {
            placeBlock2( i, j, k, false, outputFile );
            placeBlock2( i, j, -k, false, outputFile );
          }
          else
          {
            placeBlock2( i, j, k, true, outputFile );
            placeBlock2( i, j, -k, true, outputFile );

            if( i == -cubeHeight + 0.5 || i == cubeHeight - 0.5 )
            {
              for( ; k < 0; ++k )
              {
                placeBlock2( i, j, k, true, outputFile );
                placeBlock2( i, j, -k, true, outputFile );
              }
              if( k == 0 )
              {
                placeBlock2( i, j, k, true, outputFile );
              }
            }
            else if( k == -cubeHeight && usePoint2( i, j, k + 0.5, radius, true ) )
            {
              placeBlock2( i, j, k+1, true, outputFile );
              placeBlock2( i, j, -k-1, true, outputFile );
            }
          }

          break;
        }
      }
    }
  }
}

void RunSphereGenerator( string outputSphere, int type, double radius )
{
    ofstream outputFile( outputSphere );

    switch( type )
    {
      case 0:
        lowellSphere( radius, outputFile );
        break;
      case 1:
        outwardSphere( radius, outputFile );
        break;
      case 2:
        blockSphere( radius, false, outputFile );
        break;
      case 3:
        blockSphere( radius, true, outputFile );
        break;
    }

    outputFile.close();
}