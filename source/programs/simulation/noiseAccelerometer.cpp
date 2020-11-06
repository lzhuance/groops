/***********************************************/
/**
* @file noiseAccelerometer.cpp
*
* @brief Add noise and bias to accelerometer data.
*
* @author Torsten Mayer-Guerr
* @date 2005-01-21
*/
/***********************************************/

// Latex documentation
#define DOCSTRING docstring
static const char *docstring = R"(
This program adds noise and biases to simulated \file{accelerometer data}{instrument}
generated by \program{SimulateAccelerometer}.
See \configClass{noiseGenerator}{noiseGeneratorType} for details on noise generation.
)";

/***********************************************/

#include "programs/program.h"
#include "files/fileInstrument.h"
#include "classes/noiseGenerator/noiseGenerator.h"

/***** CLASS ***********************************/

/** @brief Add noise and bias to accelerometer data.
* @ingroup programsGroup */
class NoiseAccelerometer
{
public:
  void run(Config &config);
};

GROOPS_REGISTER_PROGRAM(NoiseAccelerometer, PARALLEL, "add noise and bias to accelerometer data", Simulation, Noise, Instrument)

/***********************************************/

void NoiseAccelerometer::run(Config &config)
{
  try
  {
    FileName          accelerometerInName, accelerometerOutName;
    NoiseGeneratorPtr noiseGeneratorAlong, noiseGeneratorCross, noiseGeneratorRadial;
    Vector3d          bias;

    readConfig(config, "outputfileAccelerometer", accelerometerOutName, Config::MUSTSET,  "",  "");
    readConfig(config, "inputfileAccelerometer",  accelerometerInName,  Config::MUSTSET,  "",  "");
    readConfig(config, "biasAlong",               bias.x(),             Config::DEFAULT,  "0", "[m/s**2]");
    readConfig(config, "biasCross",               bias.y(),             Config::DEFAULT,  "0", "[m/s**2]");
    readConfig(config, "biasRadial",              bias.z(),             Config::DEFAULT,  "0", "[m/s**2]");
    readConfig(config, "noiseAlong",              noiseGeneratorAlong,  Config::DEFAULT,  "",  "[m/s**2]");
    readConfig(config, "noiseCross",              noiseGeneratorCross,  Config::DEFAULT,  "",  "[m/s**2]");
    readConfig(config, "noiseRadial",             noiseGeneratorRadial, Config::DEFAULT,  "",  "[m/s**2]");
    if(isCreateSchema(config)) return;

    logStatus<<"add noise to accelerometer data <"<<accelerometerInName<<">"<<Log::endl;
    InstrumentFile accFile(accelerometerInName);
    std::vector<Arc> arcList(accFile.arcCount());

    Parallel::forEach(arcList, [&](UInt arcNo)
    {
      AccelerometerArc acc = accFile.readArc(arcNo);
      const Vector eX = noiseGeneratorAlong->noise(acc.size());
      const Vector eY = noiseGeneratorCross->noise(acc.size());
      const Vector eZ = noiseGeneratorRadial->noise(acc.size());
      for(UInt i=0; i<acc.size(); i++)
        acc.at(i).acceleration += bias + Vector3d(eX(i), eY(i), eZ(i));
      return acc;
    });

    if(Parallel::isMaster())
    {
      logStatus<<"write accelerometer data to file <"<<accelerometerOutName<<">"<<Log::endl;
      InstrumentFile::write(accelerometerOutName, arcList);
    }
  }
  catch(std::exception &e)
  {
    GROOPS_RETHROW(e)
  }
}

/***********************************************/