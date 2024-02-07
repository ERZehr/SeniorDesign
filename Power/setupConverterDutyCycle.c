/**
  ******************************************************************************
  * @file    setupConverterDutyCycle.c
  * @author  Evan Zehr
  * @version V1.0
  * @date    Jan 21, 2024
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include <stdint.h>

void enable_ports()
{
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; // turn on port B
  GPIOB->MODER &= 0xFFFFFFFC; // clear GPIOB register
  GPIOB->MODER |= 0x00000001; // set bit 1 to output
}

void TIM6_DAC_IRQHandler()
{
  TIM6->SR &= ~TIM_SR_UIF;
  int16_t readPC8_ODR;
  readPC8_ODR = 0x0100 & GPIOC->ODR;
  if(readPC8_ODR)
  {
    GPIOC->BSRR |= 0x01000000; // reset PC8_ODR
  }
  else
  {
    GPIOC->BSRR |= 0x00000100; // set PC8_ODR
  }
}

void setup_tim6()
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; // turn on timer 6
  TIM6->PSC = 47999; // 48000 - 1
  TIM6->ARR = 499; // 500 - 1
  TIM6->DIER &= ~TIM_DIER_UIE; //clear
  TIM6->DIER |= TIM_DIER_UIE; //set
  TIM6->CR1 &= ~TIM_CR1_CEN; // clear
  TIM6->CR1 |= TIM_CR1_CEN; // set
  NVIC_EnableIRQ(TIM6_DAC_IRQn); // turn on TIM6 in NVIC ISER
}


int main(void)
{
    enable_ports();
    setup_tim6();
}