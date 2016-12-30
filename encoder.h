#ifndef ENCODER_H
#define ENCODER_H

#include <ClickEncoder.h>
#include <TimerOne.h>

class Encoder
{
private:
  Encoder()
          : _encoder(0)
          , _pred(11)
          , _pgreen(3)
          , _pblue(5)
          , _last(-1)
          , _value(-1)
          , _longPress(false)
          , _cbTimer(0)
          , _cbTurnLeft(0)
          , _cbTurnRight(0)
          , _cbLongPress(0)
          , _cbClick(0)
          , _cbDoubleClick(0)
        {
          _encoder = new ClickEncoder(A1, A0, A2, 4, LOW, HIGH);
          
          Timer1.initialize(1000);
          Timer1.attachInterrupt(Encoder::timerIsr);
        }

  Encoder(const Encoder&);
  
public:
  enum CB
  {
      TIMER, LEFT, RIGHT, LPRESS, CLICK, DCLICK
  };

  ~Encoder
        {
          delete _encoder;
        }
  
  void timeout()
        {
          _encoder->service();
          if(_cbTimer) _cbTimer();
        }

  void update()
        {
          _value += _encoder->getValue();
          if(_value != _last) {
            if(_value < _last)
                call(LEFT);
            else
                call(RIGHT);
            _last = _value;
          }
  
          ClickEncoder::Button b = _encoder->getButton();
          if(b != ClickEncoder::Open) {
            switch(b) {
            case ClickEncoder::Held:
              if(!_longPress) call(LPRESS);
              _longPress = true;
              break;
            case ClickEncoder::Released:
              _longPress = false;
              break;
            case ClickEncoder::Clicked:
              call(CLICK);
              break;
            case ClickEncoder::DoubleClicked:
              call(DCLICK);
              break;
            default:
              break;
            }
          }
        }

  void setCallback(CB type, void (*cb)())
        {
          switch(type)
          {
          case(TIMER):
            _cbTimer = cb;
            break;
          case(LEFT):
            _cbTurnLeft = cb;
            break;
          case(RIGHT):
            _cbTurnRight = cb;
            break;
          case(LPRESS):
            _cbLongPress = cb;
            break;
          case(CLICK):
            _cbClick = cb;
            break;
          case(DCLICK):
            _cbDoubleClick = cb;
            break;
          default:
            break;
          }
        }

  void call(CB type)
        {
          void (*cb)() = 0;
          switch(type)
          {
          case(TIMER):
            cb = _cbTimer;
            break;
          case(LEFT):
            cb = _cbTurnLeft;
            break;
          case(RIGHT):
            cb = _cbTurnRight;
            break;
          case(LPRESS):
            cb = _cbLongPress;
            break;
          case(CLICK):
            cb = _cbClick;
            break;
          case(DCLICK):
            cb = _cbDoubleClick;
            break;
          default:
            break;
          }
          if(cb) cb();
        }

  void setPins( uint8_t r, uint8_t g, uint8_t b)
        {
          _pred = r;
          _pgreen = g;
          _pblue = b;
        }
  
  void setLED( uint8_t r, uint8_t g, uint8_t b)
        {
          analogWrite(_pred,   255-r);
          analogWrite(_pgreen, 255-g);
          analogWrite(_pblue,  255-b);
        }

  static void timerIsr()
        {
          Encoder::instance()->timeout();
        }

  static Encoder* instance()
        {
          if(!s_encoder) s_encoder = new Encoder;
          return s_encoder;
        }
  
private:
  ClickEncoder* _encoder;
  uint8_t _pred, _pgreen, _pblue;
  int16_t _last, _value;
  bool _longPress;

  void (*_cbTimer)();
  void (*_cbTurnLeft)();
  void (*_cbTurnRight)();
  void (*_cbLongPress)();
  void (*_cbClick)();
  void (*_cbDoubleClick)();

  static Encoder* s_encoder;
};

#endif
