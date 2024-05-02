#ifndef ADSR_H
#define ADSR_H

enum envState { env_idle = 0, env_attack, env_decay, env_sustain, env_release };

#define MAX_LEVEL __INT16_MAX__

class ADSR {
public:
  ADSR(){};

  int process();
  void gate(bool on);

  void setAttackRate(int rate);
  void setDecayRate(int rate);
  void setSustainLevel(int level);
  void setReleaseRate(int rate);
  void reset();
  envState getState() { return state; };

private:
  int attackIncrement;
  int decayIncrement;
  int sustainLevel;
  int releaseIncrement;

  int output = 0;
  envState state;
};

#endif