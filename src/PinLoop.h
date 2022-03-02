
class PinLoop
{

private:
  int nrSetPini,  contactPinOut, inputState;
 
public:
  bool resetInput = true;
  elapsedMillis swon;

  PinLoop(int nrPin, int Out)
  {
    resetInput = false;
    inputState = pinOut[nrSetPini].sw;
    nrSetPini = nrPin;
    contactPinOut = Out;
  }
  void loopPin()
  {
    if ((pinOut[nrSetPini].type == 0) || pinOut[nrSetPini].delayOn == 0)
    {

      if (pinOut[nrSetPini].sw == 1)
      {
        digitalWrite(contactPinOut, HIGH);
      }
      else if (pinOut[nrSetPini].sw == 0)
      {
        digitalWrite(contactPinOut, LOW);

      }
    }
    else if (pinOut[nrSetPini].type == 1)
    {
      if (pinOut[nrSetPini].sw == 1 && pinOut[nrSetPini].delayOn > 0)
      {
        if (swon >= pinOut[nrSetPini].delayOn * 1000)
        {
          pinOut[nrSetPini].sw = 0;
          digitalWrite(contactPinOut, LOW);
          wsTextAll(settingsToJson());
        }
        else
        {
          digitalWrite(contactPinOut, HIGH);
        }
      }
      else
      {
        digitalWrite(contactPinOut, LOW);
        swon = 0;
      }
    }
  }
};
PinLoop pin1(1, PIN_SW[1]);
PinLoop pin2(2, PIN_SW[2]);
