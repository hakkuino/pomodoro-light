#include <Arduino.h>
#include <Command.h>
#include <Logger.h>
#include <SmartButton.h>
#include <RgbLedAnimation.h>


/* Broches */
const byte PIN_LED_R  = PD6;
const byte PIN_LED_G  = PD5;
const byte PIN_LED_B  = PD3;

RgbLed led;

SmartButton button = SmartButton(9,8);

enum State {iAmFree = 0, iAmAlmostFree = 1, iAmBusy = 2};

State currentState = iAmFree;

void releasedCallback()
{
  
}

void changeState()
{
  if (led.isActive())
  {
    led.stopAnimation();
  }
  switch(currentState){
    case iAmFree:
    {
      currentState = iAmAlmostFree;

      RgbLedAnimationStep* steps[]={
        new RFadeTo(100, Color(255,165,0)),
        new RSetTo(100, Color(255,165,0)),
        new RStop()
      };
      led.setAnimation(sizeof(steps)/sizeof(RgbLedAnimationStep*), steps);
      led.startAnimation();

      break;
    }
    case iAmAlmostFree:
    {
      currentState = iAmBusy;

      RgbLedAnimationStep* steps[]={
        new RFadeTo(100, Color(255,0,0)),
        new RSetTo(100, Color(255,0,0)),
        new RWait(5000),
        new RCallback(changeState),
        new RStop()
      };
      led.setAnimation(sizeof(steps)/sizeof(RgbLedAnimationStep*), steps);
      led.startAnimation();

      break;
    }
    case iAmBusy:
    {
      currentState = iAmFree;

      RgbLedAnimationStep* steps[]={
        new RFadeTo(100, Color(0,255,0)),
        new RSetTo(100, Color(0,255,0)),
        new RStop()
      };
      led.setAnimation(sizeof(steps)/sizeof(RgbLedAnimationStep*), steps);
      led.startAnimation();

      break;
    }
  }
}

void alarm()
{
  currentState=iAmBusy;
  if (led.isActive())
  {
    led.stopAnimation();
  }

  RgbLedAnimationStep* steps[]={
    new RSetTo(100, Color(0,0,0)),
    new RFadeTo(300, Color(255,0,0)),
    new RWait(800),
    new RFadeTo(400, Color(0,0,0)),
    new RLoopTo(0, -1)
  };
  led.setAnimation(sizeof(steps)/sizeof(RgbLedAnimationStep*), steps);
  led.startAnimation();
}

void rainbow()
{
  currentState=iAmBusy;
  RgbLedAnimationStep* steps[]={
    new RSetTo(100, Color(0,0,0)),
    new RFadeTo(1000, Color(0,0,255)),
    new RFadeTo(1000, Color(255,0,0)),
    new RFadeTo(1000, Color(0,255,0)),
    new RFadeTo(1000, Color(0,0,255)),
    new RLoopTo(1100, -1)
  };
  led.setAnimation(sizeof(steps)/sizeof(RgbLedAnimationStep*), steps);
  led.startAnimation();  
}

void clickCallback(byte nbClicks)
{
  if (nbClicks == 1)
  {
    changeState();
  }
  else if (nbClicks == 2)
  {
    alarm();
  }
  else if (nbClicks == 3)
  {
    rainbow();
  }
  DEBUG("click " + String(nbClicks));
}

void longPressCallback(byte nbClicks, unsigned int time)
{
  boolean wasAnimated = false;
  if (led.isActive())
  {
    led.stopAnimation();
    wasAnimated=true;
  }
  if (nbClicks == 1)
  {
    float f = constrain(led.getBrightness() + 0.01, 0.1, 1.0);
    led.setBrightness(f);
  }
  else if (nbClicks == 2)
  {
    float f = constrain(led.getBrightness() - 0.01, 0.1, 1.0);
    led.setBrightness(f);
  }
  if (wasAnimated)
  {
    led.resumeAnimation();
  }
  DEBUG("longPress " + String(nbClicks) + " - " + String(time));
}

void setup() {
  Serial.begin(9600);
  led.init(PIN_LED_R,PIN_LED_G,PIN_LED_B);
  RgbLedAnimationStep* steps[]={
    new RSetTo(100,Color(0,0,0)),
    new RFadeTo(200, Color(0,255,0)),
    new RWait(300),
    new RFadeTo(200, Color(0,0,0)),
    new RWait(200),
    new RFadeTo(500, Color(0,255,0)),
    new RStop()
  };
  led.setAnimation(sizeof(steps)/sizeof(RgbLedAnimationStep*), steps);
  led.startAnimation();
  button.setClickCallback(clickCallback);
  button.setLongPressCallback(longPressCallback);
  button.setReleasedCallback(releasedCallback);
}


void loop() {
  button.tick();

  if (Serial.available() > 0)
  {
    String s = Serial.readString();
    Command command = Command(s);
    DEBUG("Command received. " + command.getCommand());
    for (int i=0; i<command.getArgsSize(); i++)
    {
      DEBUG("Arg " + String(i) + " : " + command.getArg(i));
    }
    if (command.getCommand() == "calibrate") {
      int red = command.getArg(0).toInt();
      int green = command.getArg(1).toInt();
      int blue = command.getArg(2).toInt();
     
      red = constrain(red, 0, 255);
      green = constrain(green, 0, 255);
      blue = constrain(blue, 0, 255);

      led.setCalibration(Color(red,green,blue));
    }
    else if (command.getCommand() == "rgb") {
      int red = command.getArg(0).toInt();
      int green = command.getArg(1).toInt();
      int blue = command.getArg(2).toInt();
     
      red = constrain(red, 0, 255);
      green = constrain(green, 0, 255);
      blue = constrain(blue, 0, 255);
  
      Color c = Color();
      c.setRGB(red,green,blue);
      //c.setGamma(true);
  
      led.setColor(c);
    }
    else if (command.getCommand() == "alarm")
    {
      alarm();
    }
    else if (command.getCommand() == "b")
    {
      int brightness = command.getArg(0).toInt();
      led.setBrightness(brightness/100.00);
    }
    else if (command.getCommand() == "rainbow")
    {
      rainbow();
    }
    else if (command.getCommand() == "hex")
    {
      led.setColor(Color(command.getArg(0)));
    }
    else if (command.getCommand() == "coucou")
    {
      Serial.println("tu veux voir ma light ?");
    }
    else if (command.getCommand() == "addLogFilter")
    {
      Logger::getLogger()->getFilters()->push_back(command.getArg(0));
    }
    else if (command.getCommand() == "removeAllLogFilters")
    {
      Logger::getLogger()->getFilters()->clear();
    }
  }
  
  led.runAnimation();

}