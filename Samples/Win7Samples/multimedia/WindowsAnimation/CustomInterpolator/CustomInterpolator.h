// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "UIAnimationHelper.h"

#define _USE_MATH_DEFINES
#include <math.h>

// Attract interpolator 

class CAttractInterpolator :
    public CUIAnimationInterpolatorBase<CAttractInterpolator>
{
public:

    // IUIAnimationInterpolator

    // Sets the interpolator's initial value and velocity
    
    IFACEMETHODIMP
    SetInitialValueAndVelocity
    (
        DOUBLE initialValue,                                        // The initial value
        DOUBLE initialVelocity                                      // The initial velocity
    )
    {
        m_initialValue = initialValue;
        m_initialVelocity = initialVelocity;

        m_acceleration = (m_finalValue > m_initialValue ? m_absoluteAcceleration : -m_absoluteAcceleration);

        m_duration =
            (-m_initialVelocity + (m_acceleration < 0.0 ? -1.0 : 1.0) *
                sqrt((m_initialVelocity * m_initialVelocity) + (2.0 * m_acceleration * (m_finalValue - m_initialValue))))
            / m_acceleration;

        return S_OK;
    }

    // Sets the interpolator's duration

    IFACEMETHODIMP
    SetDuration
    (
        UI_ANIMATION_SECONDS duration                               // The interpolator duration
    )
    {
        // This should not be called, since no duration dependencies were declared

        return E_FAIL;            
    }

    // Gets the interpolator's duration

    IFACEMETHODIMP
    GetDuration
    (
        UI_ANIMATION_SECONDS *pDuration                             // The interpolator duration
    )
    {   
        *pDuration = m_duration;
    
        return S_OK;
    }

    // Gets the final value to which the interpolator leads

    IFACEMETHODIMP
    GetFinalValue
    (
        DOUBLE *pValue                                              // The final value
    )
    {        
        *pValue = m_finalValue;
        
        return S_OK;
    }

    // Interpolates the value at a given offset

    IFACEMETHODIMP
    InterpolateValue
    (
        UI_ANIMATION_SECONDS offset,                                // The offset
        DOUBLE *pValue                                              // The interpolated value
    )
    {
        *pValue = (0.5 * m_acceleration * (offset * offset)) + (m_initialVelocity * offset) + m_initialValue;

        return S_OK;
    }

    // Interpolates the velocity at a given offset

    IFACEMETHODIMP
    InterpolateVelocity
    (
        UI_ANIMATION_SECONDS offset,                                // The offset
        DOUBLE *pVelocity                                           // The interpolated velocity
    )
    {
        *pVelocity = (m_acceleration * offset) + m_initialVelocity;

        return S_OK;
    }

    // Gets the interpolator's dependencies

    IFACEMETHODIMP
    GetDependencies
    (
        UI_ANIMATION_DEPENDENCIES *pDependenciesInitialValue,       // The aspects of the interpolator that depend on its initial value
        UI_ANIMATION_DEPENDENCIES *pDependenciesInitialVelocity,    // The aspects of the interpolator that depend on its initial velocity
        UI_ANIMATION_DEPENDENCIES *pDependenciesDuration            // The aspects of the interpolator that depend on its duration
    )
    {
        // The final value of the interpolator is not affected by the initial value or velocity, but
        // the intermediate values, final velocity and duration all are affected
    
        *pDependenciesInitialValue =
            UI_ANIMATION_DEPENDENCY_INTERMEDIATE_VALUES |
            UI_ANIMATION_DEPENDENCY_FINAL_VELOCITY |
            UI_ANIMATION_DEPENDENCY_DURATION;

        *pDependenciesInitialVelocity =
            UI_ANIMATION_DEPENDENCY_INTERMEDIATE_VALUES |
            UI_ANIMATION_DEPENDENCY_FINAL_VELOCITY |
            UI_ANIMATION_DEPENDENCY_DURATION;

        // This interpolator does not have a duration parameter, so SetDuration should not be called on it
        
        *pDependenciesDuration = UI_ANIMATION_DEPENDENCY_NONE;
        
        return S_OK;
    }

    // Creates an attract transition

    static HRESULT STDMETHODCALLTYPE CreateTransition(
        IUIAnimationTransitionFactory *pTransitionFactory,          // The transition factory to use to wrap the interpolator
        DOUBLE finalValue,                                          // The final value this transition leads to when applied to an animation variable
        DOUBLE acceleration,                                        // The rate of change of the velocity
        IUIAnimationTransition **ppTransition                       // The new attract transition
        )
    {
        if (acceleration == 0.0)
        {
            return E_INVALIDARG;
        }
    
        IUIAnimationInterpolator *pInterpolator;
        CAttractInterpolator *pAttractInterpolator;
        CreateInstance(&pInterpolator, &pAttractInterpolator);
        
        pAttractInterpolator->m_absoluteAcceleration = fabs(acceleration);
        pAttractInterpolator->m_finalValue = finalValue;

        HRESULT hr = pTransitionFactory->CreateTransition(pInterpolator, ppTransition);

        return hr;
    }
    
protected:

    CAttractInterpolator()
      : m_initialValue(0.0),
        m_finalValue(0.0),
        m_initialVelocity(0.0),
        m_absoluteAcceleration(0.0),
        m_acceleration(0.0)
    {
    }

    UI_ANIMATION_SECONDS m_duration;                                // Duration of the transition
    DOUBLE m_initialValue;                                          // Initial value of the transition
    DOUBLE m_finalValue;                                            // Final value of the transition
    DOUBLE m_initialVelocity;                                       // Initial velocity of the transition
    DOUBLE m_absoluteAcceleration;                                  // The original acceleration passed to the interpolator
    DOUBLE m_acceleration;                                          // Acceleration of the transition
};
