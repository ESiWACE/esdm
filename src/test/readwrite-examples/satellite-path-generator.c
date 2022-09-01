#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  const int sampleCount = 6*60*24;  //currently one day, adjust this to generate longer/shorter runs

  const int samplesPerOrbit = 6*90;
  const int samplesPerTimeslice = 60;

  const int timeSlices = 6*24 + 1;  //add the first timeslice of the next day so we can continue fetching two time slices up to 23:59
  const int heightLevels = 57;
  const int latitudes = 72;
  const int longitudes = 144;

  for(int sample = 0; sample < sampleCount; sample++) {
    double longitude = sample%samplesPerOrbit / (double)samplesPerOrbit;
    double latitude = 0.5 + 0.4*sin(longitude*2*M_PI);
    double time = sample / (double)((timeSlices - 1)*samplesPerTimeslice);
    double height = rand()/(double)RAND_MAX;

    int t = time*(timeSlices - 1);
    int h = height*(heightLevels - 1);
    int lat = latitude*(latitudes - 1);
    int lon1 = longitude*longitudes;
    int lon2 = (lon1 + 1)%longitudes;

    assert(0 <= t && t < timeSlices - 1);
    assert(0 <= h && h < heightLevels - 1);
    assert(0 <= lat && lat < latitudes - 1);
    assert(0 <= lon1 && lon1 < longitudes);
    assert(0 <= lon2 && lon2 < longitudes);

    printf("var1(4): (%d,0,%d,%d) (2,57,2,1) (2,57,2,1)\n", t, lat, lon1);
    printf("var1(4): (%d,0,%d,%d) (2,57,2,1) (2,57,2,1)\n", t, lat, lon2);
    printf("var2(4): (%d,%d,%d,%d) (2,2,2,1) (2,2,2,1)\n", t, h, lat, lon1);
    printf("var2(4): (%d,%d,%d,%d) (2,2,2,1) (2,2,2,1)\n", t, h, lat, lon2);
  }
}
