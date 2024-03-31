
#include "shader_defines.glsl"
// https://www.shadertoy.com/view/wtcczf

//Sine
float EaseInSine(float x)
{
    return 1.0 - cos((x * PI) / 2.0);
}

float EaseOutSine(float x)
{
    return sin((x * PI) / 2.0);
}

float EaseInOutSine(float x)
{ 
    return -(cos(PI * x) - 1.0) / 2.0;
}

//Quad
float EaseInQuad(float x)
{
   return x * x;
}

float EaseOutQuad(float x)
{
  return 1.0 - (1.0-x) * (1.0 -x );
}

float EaseInOutQuad(float x)
{
   //x < 0.5f ? 2 * x* x : 1 - pow(-2 * x + 2,2) /2;
   float inValue = 2.0 * x  *x;
   float outValue = 1.0- pow(-2.0 * x + 2.0,2.0) / 2.0;
   float inStep = step(inValue,0.5) * inValue;
   float outStep = step(0.5 , outValue ) * outValue;
   
   return inStep + outStep;
}

//Cubic
float EaseInCubic(float x)
{
   return x * x * x;
}

float EaseOutCubic(float x)
{ 
   return 1.0 - pow(1.0 - x,3.0);
}

float EaseInOutCubic(float x)
{
  //x < 0.5f ? 4.0f * x * x * x : 1 - Mathf.Pow(-2 *x + 2,3)/2;
  float inValue = 4.0 * x * x * x;
  float outValue = 1.0 -pow(-2.0 * x + 2.0 ,3.0) /2.0;
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}

//Quart
float EaseInQuart(float x)
{
 return x * x * x * x;
}

float EaseOutQuart(float x)
{
  return 1.0 - pow(1.0 -x, 4.0);
}

float EaseInOutQuart(float x)
{
  //x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
  float inValue = 8.0 * x * x * x * x;
  float outValue = 1.0 - pow(-2.0 * x + 2.0, 4.0) / 2.0;
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}

//Quint
float EaseInQuint(float x)
{
   return x * x * x * x * x;
}

float EaseOutQuint(float x)
{
   return 1.0 - pow(1.0 - x , 5.0);
}

float EaseInOutQuint(float x)
{
  //  x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
  float inValue = 16.0 * x * x * x * x * x;
  float outValue = 1.0 - pow(-2.0 * x + 2.0, 5.0) / 2.0;
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}

//Expo
float EaseInExpo(float x)
{
  return pow(2.0, 10.0 * x - 10.0);
}

float EaseOutExpo(float x)
{  
  return 1.0 - pow(2.0, -10.0 * x);;
}

float EaseInOutExpo(float x)
{
  float inValue = pow(2.0, 20.0 * x - 10.0) / 2.0;
  float outValue = (2.0 - pow(2.0, -20.0 * x + 10.0)) / 2.0;
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}

//Circ
float EaseInCirc(float x)
{
 return 1.0 - sqrt(1.0 - pow(x, 2.0));
}

float EaseOutCirc(float x)
{
 return sqrt(1.0 - pow(x - 1.0, 2.0));
}

float EaseInOutCirc(float x)
{
  //x < 0.5 ? (1 - sqrt(1 - pow(2 * x, 2))) / 2 : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
  float inValue = (1.0 - sqrt(1.0 - pow(2.0 * x, 2.0))) / 2.0 * step(x,0.5);
  float outValue = (sqrt(1.0 - pow(-2.0 * x + 2.0, 2.0)) + 1.0) / 2.0 * step(0.5,x);
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}

//Back
float backC1 = 1.70158;
float backC2 = 2.5949095;
float backC3 = 2.70158;

float EaseInBack(float x)
{
  return backC3 * x * x * x - backC1 * x * x;
}

float EaseOutBack(float x)
{
   return  1.0 - backC3 * pow(x - 1.0, 3.0) + backC1 * pow(x - 1.0, 2.0);
}

float EaseInOutBack(float x){
 //x < 0.5 ? (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2: (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
  float inValue = (pow(2.0 * x, 2.0) * ((backC2 + 1.0) * 2.0 * x - backC2)) / 2.0;
  float outValue = (pow(2.0 * x - 2.0, 2.0) * ((backC2 + 1.0) * (x * 2.0 - 2.0) + backC2) + 2.0) / 2.0;
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
  
}

//Elastic
float EaseInElastic(float x)
{
 float c4 = (2.0 * PI) / 3.0;
 return -pow(2.0, 10.0 * x - 10.0) * sin((x * 10.0 - 10.75) * c4);
}

float EaseOutElastic(float x)
{
 float c4 = (2.0 * PI) / 3.0;
 return pow(2.0, -10.0 * x) * sin((x * 10.0 - 0.75) * c4) + 1.0;;
}

float EaseInOutElastic(float x)
{
//x < 0.5 ? -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2: (pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 + 1;
  float c5 = (2.0 * PI) / 4.5;
  float inValue = ( -(pow(2.0, 20.0 * x - 10.0) * sin((20.0 * x - 11.125) * c5)) / 2.0 )* step(x,0.5);
  float outValue = ((pow(2.0, -20.0 * x + 10.0) * sin((20.0 * x - 11.125) * c5)) / 2.0 + 1.0 )* step(0.5,x);
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}

//Bounce
float EaseOutBounce(float x)
{
  float n1 = 7.5625;
  float d1 = 2.75;
  float step1 = step(x,1.0/d1);
  float step2 = step(x, 2.0 /d1) - step1;
  float step3 = step(x, 2.5 / d1) - step2 - step1;
  float step4 = 1.0 - step3 - step2 - step1;
   
  float value = x;
  float p0 = (n1 * x * x)* step1 ;
  value = x - (1.5 / d1);
  float p1 = (n1 * value * value + 0.75) * step2 ;
  value = x - (2.25 / d1);
  float p2 = (n1 * value * value + 0.9375) * step3;
  value = x - (2.625  / d1);
  float p3 = (n1 * value * value + 0.984375) * step4;
  return p0 + p1 + p2 + p3;
}

float EaseInBounce(float x)
{
  return 1.0 - EaseOutBounce(1.0 - x);
}

float EaseInOutBounce(float x)
{
//x < 0.5 ? (1 - easeOutBounce(1 - 2 * x)) / 2: (1 + easeOutBounce(2 * x - 1)) / 2;
  float inValue = (1.0 - EaseOutBounce(1.0 - 2.0 * x)) / 2.0 * step(x,0.5);
  float outValue =  (1.0 + EaseOutBounce(2.0 * x - 1.0)) / 2.0 * step(0.5,x);
  return step(inValue , 0.5) * inValue + step(0.5,outValue) * outValue;
}