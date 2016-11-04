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
         action-size 40
         action-count 7
         action-margin (* 0.4 (- (/ total-width 7) 40))
         entry-width 72
         entry-height 36
         input-height (* 6 entry-height)
         code-height (- total-height input-height entry-height)
         stack-width (- total-width entry-width)]
     {"body"
      {:margin 0 :padding 0}
      ".actions"
      {:display :inline-block
       :bottom 0
       :position :fixed
       :background :blue
       }
      ".actions > img"
      {
       :width (+ action-size (* action-margin 2))
       :height action-size
       :margin-right action-margin
       :margin-left action-margin
       }
      ".current"
      {:background "#eee"}
      "div"
      {:margin 0
       :padding 0}
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
    :width 60
    :height :40
    :style
    {
     :padding "0 10px 0 10px"
     }
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

(defn object-view []
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
    :else [:div "obj"]))
(defn actions []
  [:div.actions
   #_[action-button 593402
    (fn []
      (db! [:ui :current] (count (db [:data] [])))
      (db! [:ui :input] :number))]
   [action-button 605398 #(js/console.log "layouts")]
   [action-button "47250_num" #(js/console.log "num")]
   [action-button 47250 #(js/console.log "string")]
   [action-button "209279_rotate" #(js/console.log "fn")]
   [action-button 593402 #(js/console.log "world")]
   [action-button 684642 #(js/console.log "delete")]
   [action-button 619343 #(js/console.log "ok")]
   [:button
    {:on-click
     (fn []
       (db! [:ui :current] (count (db [:data] [])))
       (db! [:ui :input] :number))}
    "123.."]
   [:button
    {:on-click 
    (fn []
      (db! [:ui :current] (count (db [:data] [])))
      (db! [:ui :input] :string))}
    "abc.."]])
(defn data-view []
  (into
   [:div]
   (reverse
    (map-indexed
     (fn [i o]
       [:div.entry
        {:on-click #(db! [:ui :current] i)
         :class (if (= i (db [:ui :current])) "current" "")}
        (JSON.stringify (clj->js (get o :code ""))) [:br] (str (get o :val ""))])
     (db [:data] [])))))
(into
 [:div]
 (reverse
  (map-indexed
   (fn [i o]
     [:div.entry (str i) [:br] (str o)])
   (db [:data] []))
  #_(for [o (db [:data] [])]
      [:div.entry
       (JSON.stringify (clj->js (get o :code)))
       [:br]
       (str (get o :val))])))
(defn fn-list [o]
   ;[:div.fn-list (str (.-constructor (o)))]
  [:div (str (remove #{"constructor"}(js->clj (js/Object.getOwnPropertyNames (.-prototype (.-constructor o))))))]
   )
(defn prop-list [o]
  [:div.fn-list
   (str (js->clj (js/Object.getOwnPropertyNames o)))
   ])
   
(defn main []
  (let [obj (db [:data (db [:ui :current] -1)] {})
        val (get obj :val #js{})]
    (log 'here obj val)
   [:div
    [object-view obj]
    [fn-list val]
    [prop-list val]
    [actions]
    [data-view]])
  )
(render
 [main]
 )
