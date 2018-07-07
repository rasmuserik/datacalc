import { createStore, applyMiddleware, compose } from 'redux'
import thunk from 'redux-thunk'
import Immutable from 'immutable'

const initialState = Immutable.fromJS({
  hello: 'world'
});
// Actions
export const someAction = () => async (dispatch, getState) => {

  // dispatch request started

  //do something

  // dispatch result
}

// Selectors
export const someSelector = (state) => state.get('hello');

// Reducers

const reducers = {}
reducers.ACTION = (state, action) => {
  return state;
}


// State
const composeEnhancers =
  typeof window === 'object' &&
  window.__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ ?   
    window.__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ : compose;

const middleware = [ thunk ]

export const store = createStore((state, action) => reducers[action.type] ? reducers[action.type](state, action) : state, initialState, composeEnhancers(applyMiddleware(thunk)))
