// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#ifndef __UIAnimationHelper_h__
#define __UIAnimationHelper_h__

#pragma once

#include <UIAnimation.h>

// Base class template for all UIAnimation callback base class templates
template <class IUIAnimationCallback, class CUIAnimationCallbackDerived>
class CUIAnimationCallbackBase :
    public IUIAnimationCallback
{
public:
    
    // Creates an instance of CUIAnimationCallbackDerived and returns a pointer to its IUIAnimationCallback interface,
    // plus an optional pointer to the object itself.  Note that the latter is not AddRef'ed.
    static __checkReturn HRESULT
    CreateInstance
    (
            __deref_out IUIAnimationCallback **ppUIAnimationCallback,
        __deref_opt_out CUIAnimationCallbackDerived **ppUIAnimationCallbackDerived = NULL
    ) throw()
    {
        if (ppUIAnimationCallbackDerived != NULL)
        {
            *ppUIAnimationCallbackDerived = NULL;
        }
        
        if (ppUIAnimationCallback == NULL)
        {
            return E_POINTER;
        }
        
        CUIAnimationCallbackObject<IUIAnimationCallback, CUIAnimationCallbackDerived> *pUIAnimationCallbackDerived =
            new CUIAnimationCallbackObject<IUIAnimationCallback, CUIAnimationCallbackDerived>;
        
        if (pUIAnimationCallbackDerived == NULL)
        {
            *ppUIAnimationCallback = NULL;
            return E_OUTOFMEMORY;
        }
        
        *ppUIAnimationCallback = static_cast<IUIAnimationCallback *>(pUIAnimationCallbackDerived);
        (*ppUIAnimationCallback)->AddRef();
        
        // Do not AddRef class pointer; caller must ensure it has a shorter lifetime than the interface pointer        
        if (ppUIAnimationCallbackDerived != NULL)
        {
            *ppUIAnimationCallbackDerived = pUIAnimationCallbackDerived;
        }
        
        return S_OK;
    }
    
protected:
    
    HRESULT
    QueryInterfaceCallback
    (
               __in REFIID riid,
               __in REFIID riidCallback,
        __deref_out void **ppvObject
    ) throw()
    {
        if (ppvObject == NULL)
        {
            return E_POINTER;
        }
        
        if (riid == IID_IUnknown ||
            riid == riidCallback)
        {
            *ppvObject = static_cast<IUIAnimationCallback *>(this);
            AddRef();
            return S_OK;
        }
        
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    
private:
    
    template <class IUIAnimationCallback, class CUIAnimationCallbackDerived>
    class CUIAnimationCallbackObject :
        public CUIAnimationCallbackDerived
    {
    public:
        
        CUIAnimationCallbackObject()
         : m_dwRef(0)
        {
        }
        
        // IUnknown
        
        IFACEMETHODIMP
        QueryInterface
        (
                   __in REFIID riid,
            __deref_out void **ppvObject
        )
        {
            IUIAnimationCallback **ppAnimationInterface = reinterpret_cast<IUIAnimationCallback **>(ppvObject);
            return QueryInterfaceCallback(riid, IID_PPV_ARGS(ppAnimationInterface));
        }
        
        IFACEMETHODIMP_(ULONG)
        AddRef()
        {
            return ++m_dwRef;
        }
        
        IFACEMETHODIMP_(ULONG)
        Release()
        {
            if (--m_dwRef == 0)
            {
                delete this;
                return 0;
            }
            
            return m_dwRef;
        }
        
    private:
        
        DWORD m_dwRef;
    };
};


/***************************************************************************************\

  Callback Base Class Templates
  
  To define a UIAnimation "callback object", simply derive a class from the appropriate
  template instantiation, e.g.:
  
    class CMyStoryboardEventHandler :
        public CUIAnimationStoryboardEventHandlerBase<CMyStoryboardEventHandler>
    {
        ...
        Implementations of IUIAnimationStoryboardEventHandler methods
        ...
    };
  
  Then, to create an instance of the class, call its static CreateInstance method: 
  
    IUIAnimationStoryboardEventHandler *pStoryboardEventHandler;
    hr = CMyStoryboardEventHandler::CreateInstance(
        &pStoryboardEventHandler
        );
    if (SUCCEEDED(hr))
    {
        hr = m_pStoryboard->SetStoryboardEventHandler(
            pStoryboardEventHandler
            );
        ...
    }
  
  An optional temporary class pointer can be used to initialize the object:
  
    IUIAnimationStoryboardEventHandler *pStoryboardEventHandler;
    CMyStoryboardEventHandler *pMyStoryboardEventHandler;
    hr = CMyStoryboardEventHandler::CreateInstance(
        &pStoryboardEventHandler,
        &pMyStoryboardEventHandler
        );
    if (SUCCCEEDED(hr))
    {
        pMyStoryboardEventHandler->Initialize(this);
        hr = m_pStoryboard->SetStoryboardEventHandler(
            pStoryboardEventHandler
            );
        ...
    }

\***************************************************************************************/

// Base class template for implementation of IUIAnimationManagerEventHandler
template <class CManagerEventHandlerDerived>
class CUIAnimationManagerEventHandlerBase :
    public CUIAnimationCallbackBase<IUIAnimationManagerEventHandler, CManagerEventHandlerDerived>
{
public:
    
    // IUIAnimationManagerEventHandler
    
    // Handles OnManagerStatusChanged events, which occur when the animation manager's status changes
    IFACEMETHODIMP
    OnManagerStatusChanged
    (
        __in UI_ANIMATION_MANAGER_STATUS newStatus,                     // The new status
        __in UI_ANIMATION_MANAGER_STATUS previousStatus                 // The previous status
    ) = 0;
};

// Base class template for implementation of IUIAnimationVariableChangeHandler
template <class CVariableChangeHandlerDerived>
class CUIAnimationVariableChangeHandlerBase :
    public CUIAnimationCallbackBase<IUIAnimationVariableChangeHandler, CVariableChangeHandlerDerived>
{
public:
    
    // IUIAnimationVariableChangeHandler
    
    // Handles OnValueChanged events, which occur when an animation variable's value changes; receives value updates as DOUBLE
    IFACEMETHODIMP
    OnValueChanged
    (
        __in IUIAnimationStoryboard *storyboard,                        // The storyboard that is animating the variable
        __in IUIAnimationVariable *variable,                            // The animation variable that was updated
        __in DOUBLE newValue,                                           // The new value
        __in DOUBLE previousValue                                       // The previous value
    ) = 0;
};

// Base class template for implementation of IUIAnimationVariableIntegerChangeHandler
template <class CVariableIntegerChangeHandlerDerived>
class CUIAnimationVariableIntegerChangeHandlerBase :
    public CUIAnimationCallbackBase<IUIAnimationVariableIntegerChangeHandler, CVariableIntegerChangeHandlerDerived>
{
public:
    
    // IUIAnimationVariableIntegerChangeHandler
    
    // Handles OnIntegerValueChanged events, which occur when an animation variable's rounded value changes; receives value updates as INT32
    IFACEMETHODIMP
    OnIntegerValueChanged
    (
        __in IUIAnimationStoryboard *storyboard,                        // The storyboard that is animating the variable
        __in IUIAnimationVariable *variable,                            // The animation variable that was updated
        __in INT32 newValue,                                            // The new rounded value
        __in INT32 previousValue                                        // The previous rounded value
    ) = 0;
};

// Base class template for implementation of IUIAnimationStoryboardEventHandler
template <class CStoryboardEventHandlerDerived>
class CUIAnimationStoryboardEventHandlerBase :
    public CUIAnimationCallbackBase<IUIAnimationStoryboardEventHandler, CStoryboardEventHandlerDerived>
{
public:
    
    // IUIAnimationStoryboardEventHandler
    
    // Handles OnStoryboardStatusChanged events, which occur when a storyboard's status changes
    IFACEMETHODIMP
    OnStoryboardStatusChanged
    (
        __in IUIAnimationStoryboard *storyboard,                        // The storyboard that changed status
        __in UI_ANIMATION_STORYBOARD_STATUS newStatus,                  // The new status
        __in UI_ANIMATION_STORYBOARD_STATUS previousStatus              // The previous status 
    ) = 0;
    
    // Handles OnStoryboardUpdated events, which occur when a storyboard is updated
    IFACEMETHODIMP
    OnStoryboardUpdated
    (
        __in IUIAnimationStoryboard *storyboard                         // The storyboard that was updated
    ) = 0;
};

// Base class template for implementation of IUIAnimationPriorityComparison
template <class CPriorityComparisonDerived>
class CUIAnimationPriorityComparisonBase :
    public CUIAnimationCallbackBase<IUIAnimationPriorityComparison, CPriorityComparisonDerived>
{
public:
    
    // IUIAnimationPriorityComparison
    
    // Determines the relative priority between a scheduled storyboard and a new storyboard
    IFACEMETHODIMP
    HasPriority
    (
        __in IUIAnimationStoryboard *pStoryboardScheduled,              // Currently scheduled storyboard
        __in IUIAnimationStoryboard *pStoryboardNew,                    // New storyboard that conflicts with scheduled storyboard
        __in UI_ANIMATION_PRIORITY_EFFECT priorityEffect                // Potential effect on attempt to schedule storyboard if HasPriority returns S_FALSE
    ) = 0;
};

// Base class template for implementation of IUIAnimationInterpolator
template <class CInterpolatorDerived>
class CUIAnimationInterpolatorBase :
    public CUIAnimationCallbackBase<IUIAnimationInterpolator, CInterpolatorDerived>
{
public:
    
    // IUIAnimationInterpolator
    
    // Sets the interpolator's initial value and velocity
    IFACEMETHODIMP
    SetInitialValueAndVelocity
    (
        __in DOUBLE initialValue,                                       // The initial value
        __in DOUBLE initialVelocity                                     // The initial velocity
    ) = 0;
    
    // Sets the interpolator's duration
    IFACEMETHODIMP
    SetDuration
    (
        __in UI_ANIMATION_SECONDS duration                              // The interpolator duration
    ) = 0;
    
    // Gets the interpolator's duration
    IFACEMETHODIMP
    GetDuration
    (
        __out UI_ANIMATION_SECONDS *duration                            // The interpolator duration
    ) = 0;
    
    // Gets the final value to which the interpolator leads
    IFACEMETHODIMP
    GetFinalValue
    (
        __out DOUBLE *value                                             // The final value
    ) = 0;
    
    // Interpolates the value at a given offset
    IFACEMETHODIMP
    InterpolateValue
    (
         __in UI_ANIMATION_SECONDS offset,                              // The offset
        __out DOUBLE *value                                             // The interpolated value
    ) = 0;
    
    // Interpolates the velocity at a given offset
    IFACEMETHODIMP
    InterpolateVelocity
    (
         __in UI_ANIMATION_SECONDS offset,                              // The offset
        __out DOUBLE *velocity                                          // The interpolated velocity
    ) = 0;
    
    // Gets the interpolator's dependencies
    IFACEMETHODIMP
    GetDependencies
    (
        __out UI_ANIMATION_DEPENDENCIES *initialValueDependencies,      // The aspects of the interpolator that depend on its initial value
        __out UI_ANIMATION_DEPENDENCIES *initialVelocityDependencies,   // The aspects of the interpolator that depend on its initial velocity
        __out UI_ANIMATION_DEPENDENCIES *durationDependencies           // The aspects of the interpolator that depend on its duration
    ) = 0;
};

// Base class template for implementation of IUIAnimationTimerEventHandler
template <class CTimerEventHandlerDerived>
class CUIAnimationTimerEventHandlerBase :
    public CUIAnimationCallbackBase<IUIAnimationTimerEventHandler, CTimerEventHandlerDerived>
{
public:
    
    // IUIAnimationTimerEventHandler
    
    // Handles OnPreUpdate events, which occur before an animation udpate begins
    IFACEMETHODIMP
    OnPreUpdate() = 0;
    
    // Handles OnPostUpdate events, which occur after an animation update is finished
    IFACEMETHODIMP
    OnPostUpdate() = 0;
    
    // Handles OnRenderingTooSlow events, which occur when the rendering frame rate for an animation falls below the minimum acceptable frame rate
    IFACEMETHODIMP
    OnRenderingTooSlow
    (
        __in UINT32 framesPerSecond                                     // The current frame rate, in frames-per-second
    ) = 0;
};


/***************************************************************************************\

  Helper Functions

\***************************************************************************************/

// Gets the tag of a variable, or (NULL, idDefault) if no tag was set
inline __checkReturn HRESULT
UIAnimation_GetVariableTag
(
               __in IUIAnimationVariable *pVariable,
               __in UINT32 idDefault,
    __deref_opt_out IUnknown **ppObject,
          __out_opt UINT32 *pId
)
{
    HRESULT hr = pVariable->GetTag(ppObject, pId);
    
    if (hr == UI_E_VALUE_NOT_SET)
    {
        if (ppObject != NULL)
        {
            *ppObject = NULL;
        }
        
        if (pId != NULL)
        {
            *pId = idDefault;
        }
        
        hr = S_OK;
    }
    
    return hr;
}

// Gets the tag of a storyboard, or (NULL, idDefault) if no tag was set
inline __checkReturn HRESULT
UIAnimation_GetStoryboardTag
(
               __in IUIAnimationStoryboard *pStoryboard,
               __in UINT32 idDefault,
    __deref_opt_out IUnknown **ppObject,
          __out_opt UINT32 *pId
)
{
    HRESULT hr = pStoryboard->GetTag(ppObject, pId);
    
    if (hr == UI_E_VALUE_NOT_SET)
    {
        if (ppObject != NULL)
        {
            *ppObject = NULL;
        }
        
        if (pId != NULL)
        {
            *pId = idDefault;
        }
        
        hr = S_OK;
    }
    
    return hr;
}

#endif // __UIAnimationHelper_h__
