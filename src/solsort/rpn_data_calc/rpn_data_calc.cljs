(ns solsort.rpn-data-calc.rpn-data-calc
  (:require-macros
   [cljs.core.async.macros :refer [go go-loop alt!]]
   [reagent.ratom :as ratom :refer  [reaction]])
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
         entry-width 72
         entry-height 36
         input-height (* 6 entry-height)
         code-height (- total-height input-height entry-height)
         stack-width (- total-width entry-width)
]
     {"body"
      {:margin 0 :padding 0}
      "div"
      {:margin 0
       :padding 0}
      :.val
      {:text-align :right}
      "#main"
      {:fonts-size (/ entry-height 2)
       :background-color :yellow}
      :.entry
      {:display :inline-block
       :width entry-width
       :height entry-height
       :outline "1px solid black"
       }
      "#code"
      {
       :position :fixed
       :top 0
       :left 0
       :display :inline-block
       :width entry-width
       :height code-height
       :overflow-y :auto
       :overflow-x :auto
       :scroll :auto
       :background-color :red}
      "#top"
      {:position :fixed
       :background :yellow
       :top code-height
       :left 0}
      "#object"
      {:display :inline-block
       :position :fixed
       :top 0
       :right 0
       :width (- total-width entry-width)
       :height code-height
       :background-color :yellow}
      "#input"
      {:position :fixed
       :bottom 0
       :left 0
       :line-height 10
       :background :blue
       :height input-height}
      "#input > div"
      {:display :inline-block
       :white-space :nowrap
       :line-height "100%"
       :overflow-y :auto
       :overflow-x :auto
       :height entry-height
       :width total-width
       :background-color :blue}
      "#stack"
      {:position :fixed
       :top code-height
       :right 0
       :display :inline-block
       :white-space :nowrap
       :overflow-y :auto
       :overflow-x :auto
       :height entry-height
       :width stack-width
       :background-color :green}})
   :styling))
(js/window.addEventListener "resize" styling)
(js/window.addEventListener "load" #(js/setTimeout styling 0))
(styling)

(defn code-view []
  (into
   [:div#code]
   (for [i (range 20)]
     [:div.entry
      [:div.fn "hello"]
      [:div.val i]])))
(defn stack-view []
  (into
   [:div#stack]
   (for [i (range 20)]
     [:div.entry
      [:div.fn "hello"]
      [:div.val i]])))
(defn object-view []
  [:div#object])
(defn input-view []
  [:div#input
   (into [:div] (for [i (range 100)] [:div.entry [:div.fn i] [:div.val ""]]))
   (into [:div] (for [i (range 100)] [:div.entry [:div.fn i] [:div.val ""]]))
   (into [:div] (for [i (range 100)] [:div.entry [:div.fn i] [:div.val ""]]))
   (into [:div] (for [i (range 100)] [:div.entry [:div.fn i] [:div.val ""]]))
   (into [:div] (for [i (range 100)] [:div.entry [:div.fn i] [:div.val ""]]))
   (into [:div] (for [i (range 100)] [:div.entry [:div.fn i] [:div.val ""]]))
   ]
  )
(render
 [:div
  [code-view]
  [object-view]
  [:div#top.entry
    [:div.fn "hello"]
    [:div.val 123]]
  [stack-view]
  [input-view]])
