#ifndef ADSR_H
#define ADSR_H

enum envState { env_idle = 0, env_attack, env_decay, env_sustain, env_release };

class ADSR {
public:
  ADSR(){};

  float process();
  void gate(bool on);

  void setAttackRate(int rate);
  void setDecayRate(int rate);
  void setSustainLevel(int level);
  void setReleaseRate(int rate);
  void reset();
  envState getState() { return state; };

  float maxLevel = 255;

private:
  int attackIncrement;
  int decayIncrement;
  int sustainLevel;
  float releaseIncrement;

  float output = 0;
  envState state;
};

#endif