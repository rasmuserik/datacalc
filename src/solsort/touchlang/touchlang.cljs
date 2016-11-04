(ns solsort.touchlang.touchlang
  (:require-macros
   [cljs.core.async.macros :refer [go go-loop alt!]]
   [reagent.ratom :as ratom :refer  [reaction]]
   [solsort.toolbox.macros])
  (:require
   [cljs.reader]
   [solsort.toolbox.setup]
   [solsort.toolbox.appdb :refer [db db! db-async!]]
   [solsort.toolbox.ui :refer [input select]]
   [solsort.util
    :refer
    [<ajax <seq<! js-seq load-style! put!close!
     parse-json-or-nil log page-ready render dom->clj]]
   [reagent.core :as reagent :refer []]
   [clojure.string :as string :refer [replace split blank?]]
   [cljs.core.async :refer [>! <! chan put! take! timeout close! pipe]]))


(defn styling []
  (load-style!
   (let [total-width js/window.innerWidth
         total-height js/window.innerHeight
         action-count 7
         action-size 40
         action-margin (* 0.4 (- (/ total-width action-count) action-size))
         action-top-padding 10
         action-height (+ action-size action-top-padding)
         sexpr-height 36
         h (- total-height action-height sexpr-height)
         w total-width
         ratio 0.6
         [main props fns objs]
         (map
          (fn [[x0 y0 x1 y1]] {:display :inline-block
                               :position :absolute
                               :overflow :auto
                               :left (* w x0) :top (* h y0)
                               :right (- w (* w x1)) :bottom (- h (* h y1))})
          (case (db [:ui :layout])
            [[0 0 1 (- 1 ratio)]
             [0 ratio .25 1]
             [.25 ratio .5 1]
             [.5 ratio 1 1]]))
         entry-width 72 ; 70-140
         entry-height 36
         ]
     {"body"
      {
       :background :blue
       :margin 0 :padding 0}
      ".sexpr"
      {
       :background "#ccf"
       :display :inline-block
       :position :fixed
       :width "100%"
       :top 0
       :white-space :nowrap
       ;:line-height sexpr-height
       :overflow :auto
       :padding-left 10
       :padding-right 10
       :height sexpr-height
       :box-shadow "0px 1px 4px rgba(0,0,0,0.5)"
       }
      ".actions"
      {
       :background :white
       :display :inline-block
       :bottom 0
       :padding-top action-top-padding
       :margin 0
       :left 0
       :height (+ action-size action-top-padding)
       :position :fixed
       :text-align :center
       :width "100%"
       :box-shadow "0px -1px 4px rgba(0,0,0,0.5)"
       }
      ".actions > img"
      {
       :width (+ action-size (* 2 action-margin))
       :height action-size
       :padding-right action-margin
       :padding-left action-margin
       :margin 0
       }
      ".content-container"
      {
       :background "#cfc"
       :display :inline-block
       :position :fixed
       :left 0
       :right 0
       :top sexpr-height
       :bottom action-height}
      ".props"
      (into props
       {:background :red})
      ".fns"
      (into fns
            {:background :green})
      ".main"
      (into main
            {:background :blue})
      ".objs"
      (into objs
            {:background :yellow})
      :.entry
      {:display :inline-block
       :white-space :nowrap
       :overflow :hidden
       :width entry-width
       :height entry-height
       :outline "1px solid black"}})
   :styling))
(js/window.addEventListener "resize" styling)
(js/window.addEventListener "load" #(js/setTimeout styling 0))
(styling)

(defn action-button [id f]
  [:img.icon
   {:src (str "assets/icons/noun_" id ".svg")
    :on-click f
    }
   ]
  )
(deftype Number [val]
  Object
  (add [_ y] (Number. (+ val (.-val y))))
  (sub [_ y] (Number. (- val (.-val y))))
  (mul [_ y] (Number. (* val (.-val y))))
  (toJSON [_] #js ["value" val])
  (toString [_] (str val)))

(deftype String [val]
  Object
  (toJSON [_] #js ["value" val])
  (toString [_] (str val)))

(defn sexpr []
  [:div.sexpr
   "(defn data-view [] (into [:div] (reverse (map-indexed (fn [i o] [:div.entry {:on-click #(db! [:ui :current] i) :class (if (= i (db [:ui :current])) \"current\" \"\")} (JSON.stringify (clj->js (get o :code \"\"))) [:br] (str (get o :val \"\"))]) (db [:data] [])))))"]
  )
(defn main []
  (cond
    (= (db [:ui :input]) :string)
    [:div.main
     [:form
      {:on-submit
       (fn [e]
         (.preventDefault e)
         (let [val (db [:ui :value] "")]
           (db! [:data]
                (conj (db [:data] [])
                      {:code val
                       :val val})))
         (db! [:ui :input]))}
      [:textarea
       {:auto-focus true
        :on-change
        (fn [e]
          (db! [:ui :value] (String. (-> e (.-target) (.-value)))))}]
      [:input
       {:type :submit}]]]
    (= (db [:ui :input]) :number)
    [:form
     {:on-submit
      (fn [e]
        (.preventDefault e)
        (let [val (db [:ui :value] 0)]
          (db! [:data]
               (conj (db [:data] [])
                     {:code val
                      :val val})))
        (db! [:ui :input]))}
     [:input
      {:auto-focus true
       :inputmode :numeric
       :on-change (fn [e] (db! [:ui :value] (Number. (js/parseFloat (-> e (.-target) (.-value))))))}]
     #_[:input
        {:type :submit}]]
    :else [:div "obj"]))
(defn objs []
  (into
   [:div.objs]
   (reverse
    (map-indexed
     (fn [i o]
       [:div.entry
        {:on-click #(db! [:ui :current] i)
         :class (if (= i (db [:ui :current])) "current" "")}
        (JSON.stringify (clj->js (get o :code ""))) [:br] (str (get o :val ""))])
     (db [:data] [])))))
(defn fns [o]
  [:div.fns (str (remove #{"constructor"}(js->clj (js/Object.getOwnPropertyNames (.-prototype (.-constructor o))))))]
   )
(defn props [o]
  [:div.props
   (str (js->clj (js/Object.getOwnPropertyNames o)))
   ])
(defn actions []
  [:div.actions
   #_[action-button 593402
      (fn []
        (db! [:ui :current] (count (db [:data] [])))
        (db! [:ui :input] :number))]
   [action-button 605398 #(js/console.log "layouts")]
   [action-button "47250_num"
    (fn []
      (db! [:ui :current] (count (db [:data] [])))
      (db! [:ui :input] :number))
    ]
   [action-button 47250
    (fn []
      (db! [:ui :current] (count (db [:data] [])))
      (db! [:ui :input] :string))
    ]
   [action-button "209279_rotate" #(js/console.log "fn")]
   [action-button 593402 #(js/console.log "world")]
   [action-button 684642 #(js/console.log "delete")]
   [action-button 619343 #(js/console.log "ok")]])
(defn ui []
  (let [obj (db [:data (db [:ui :current] -1)] {})
        val (get obj :val #js{})]
    (log 'here obj val)
   [:div
    [:div.content-container
     [main obj]
     [props val]
     [fns val]
     [actions]
     [objs]
     ]
    [sexpr]
    ])
  )
(render
 [ui]
 )
