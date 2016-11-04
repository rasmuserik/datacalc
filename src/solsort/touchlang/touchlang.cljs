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


(def layouts
  [[[0 .0 1 .1]
    [0 .1 1 .5]
    [0 .6 .25 1]
    [.25 .6 .5 1]
    [.5 .6 1 1]
    [0 .5 1 .6]]
   [[0 .0 1 .1]
    [.25 .1 1 .7]
    [0 .1 .25 .4]
    [0 .4 .25 .7]
    [0 .8 1 1]
    [0 .7 1 .8]]
   [[0 .0 1 .1]
    [.2 .1 .8 .9]
    [0 .1 .2 .5]
    [0 .5 .2 .9]
    [.8 .1 1 .9]
    [0 .9 1 1]]
   ])
(defn hash-color-light [s]
  (str "#"
       (-> s
           (hash)
           (bit-and 0xffffff)
           (bit-or 0x1b0b0b0)
           (.toString 16)
           (.slice 1)
           )))
(defn styling []
  (load-style!
   (let [total-width js/window.innerWidth
         total-height js/window.innerHeight
         sexpr-height 36
         h total-height
         w total-width
         ratio 0.6
         [sexpr main props fns objs actions]
         (map
          (fn [[x0 y0 x1 y1]] {:display :inline-block
                               :position :absolute
                               :overflow :auto
                               :left (* w x0) :top (* h y0)
                               :width (* w (- x1 x0))
                               :height (* h (- y1 y0))
                               :right (- w (* w x1)) :bottom (- h (* h y1))})
          (get layouts (mod (db [:ui :layout] 0) (count layouts))))
         action-count 7
         action-size (* 0.8 (min (:height actions) (/ (:width actions) action-count)))
         action-hpad (- (/ (:width actions) action-count) action-size)
         action-vpad (- (:height actions) action-size)
         entries-per-line (max 1 (js/Math.floor (/ (:width objs) 80)))
         entry-width (/ (:width objs) entries-per-line)
         entry-height (* 0.5 entry-width)
         ]
     {"body"
      {:margin 0 :padding 0}
      ".sexpr"
      (into sexpr
        {:background "#eef"
        :overflow :auto
        :padding-left 10
        :padding-right 10
        :box-shadow "2px 2px 5px rgba(0,0,0,0.5)"
        })
      ".actions"
      (into actions
        {:background :white
        :text-align :center
         :overflow :hidden
        :box-shadow "2px 2px 5px rgba(0,0,0,0.5)"
        })
      ".actions > img"
      {:width (+ action-hpad action-size)
       :height (+ action-size action-vpad)
       :padding-top (* .5 action-vpad)
       :padding-bottom (* .5 action-vpad)
       :padding-left (* .5 action-hpad)
       :padding-right (* .5 action-hpad)
       :margin 0
       }
      ".props"
      (into props
            {;:background "#000"
             :outline "1px solid black"
             })
      ".fns"
      (into fns
            {;:background "#000"
             :outline "1px solid black"
             })
      ".main"
      (into main
            {:background "#ccf"})
      ".objs"
      (into objs
            {:background "#ffc"
             :outline "1px solid black"
             })
      :.entry
      {:display :inline-block
       :white-space :nowrap
       :overflow :hidden
       :font-size (* entry-height 0.3)
       :line-height (/ entry-height 3)
       :width entry-width
       :height entry-height
       :outline "1px solid black"}})
   :styling))
(js/window.addEventListener "resize" styling)
(js/window.addEventListener "load" #(js/setTimeout styling 0))
(styling)

(deftype Number [val]
  Object
  (add [_ y] (Number. (+ val (.-val y))))
  (sub [_ y] (Number. (- val (.-val y))))
  (mul [_ y] (Number. (* val (.-val y))))
  (availableProps [_] {"inc" ["add" ["literal" 1]]
                       "dec" ["sub" ["literal" 1]]})
  (toJSON [_] #js ["value" val])
  (toString [_] (str val)))
(deftype String [val]
  Object
  (toJSON [_] #js ["value" val])
  (toString [_] (str val)))

(defn execute [expr]
  (js/console.log 'execute expr))
(defn begin-form [id]
  (js/console.log 'begin-form id))

(defn action-button [id f]
  [:img.icon
   {:src (str "assets/icons/noun_" id ".svg")
    :on-click f
    }
   ]
  )
(defn sexpr []
  [:div.sexpr
   "(defn data-view [] (into [:div] (reverse (map-indexed (fn [i o] [:div.entry {:on-click #(db! [:ui :current] i) :class (if (= i (db [:ui :current])) \"current\" \"\")} (JSON.stringify (clj->js (get o :code \"\"))) [:br] (str (get o :val \"\"))]) (db [:data] [])))))"]
  )
(defn main []
  [:div.main
  (cond
    (= (db [:ui :input]) :string)
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
       {:type :submit}]]
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
    :else [:div "obj"])])
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
  (into [:div.fns]
        (for [v
              (sort (remove #{"constructor"}
                       (js->clj (js/Object.getOwnPropertyNames
                                 (.-prototype (.-constructor o))))))]
          [:div.fn
           {:on-click #(begin-form v)
            :style
            {:background-color (hash-color-light v)}
            }
           [:strong v] [:br]
           [:em "..."]
           ]
          ))
   )
(defn availableProps [o]
  (sort
   (if (aget o "availableProps")
     (js->clj (.availableProps o))
     (for [name (js->clj (js/Object.getOwnPropertyNames o))]
       [name ["get" name]]
       ))))

(defn props [o]
  (into [:div.props]
        (for [[k v] (availableProps o)]
          [:div.fn
           {:on-click #(execute v)
            :style
            {:background-color (hash-color-light k)}
            }
           [:strong k] [:br]
           [:em (str v)]
           ]
          ))
   )

(defn actions []
  [:div.actions
   #_[action-button 593402
      (fn []
        (db! [:ui :current] (count (db [:data] [])))
        (db! [:ui :input] :number))]
   [action-button 605398
    (fn []
      (db! [:ui :layout] (inc (db [:ui :layout] 0)))
      (js/setTimeout styling 0)
       )]
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
   [:div
    [main obj]
    [props val]
    [fns val]
    [objs]
    [sexpr]
    [actions]
    ])
  )
(render
 [ui]
 )
