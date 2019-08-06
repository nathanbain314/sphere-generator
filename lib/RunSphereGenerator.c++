#include "SphereGenerator.h"
#include <tclap/CmdLine.h>

using namespace TCLAP;

int main( int argc, char **argv )
{
  try
  {
    CmdLine cmd("Generates lego spheres", ' ', "1.0");

    ValueArg<double> radiusArg( "r", "radius", "Radius of sphere", true, 0, "double", cmd );

    ValueArg<int> typeArg( "t", "type", "Type of object to generate", false, 1, "int", cmd );

    ValueArg<string> outputArg( "o", "output", "Output LDraw file (.ldr)", true, "out.ldr", "string", cmd );

    cmd.parse( argc, argv );

    string outputSphere = outputArg.getValue();
    int type            = typeArg.getValue();
    double radius       = radiusArg.getValue();

    RunSphereGenerator( outputSphere, type, radius );
  }
  catch (ArgException &e)  // catch any exceptions
  {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
  }
  return 0;
}