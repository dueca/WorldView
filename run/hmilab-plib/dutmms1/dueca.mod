;; -*-scheme-*-

;; set up switches
(define claim-visual-thread? #t)

;; in general, it is a good idea to clearly document your set up
;; this is an excellent place. 
;; node set-up
(define ecs-node 0)    ; dutmms1, send order 3
(define ig-node-left 1)
(define ig-node-right 2)

;; priority set-up
;               0 administration
;               1 ogre graphics drawing thread
;               2 simulation, unpackers
;               3 communication
;               4 ticker

; administration priority. Run the interface and logging here
(define admin-priority (make-priority-spec 0 0))

; priority of simulation, just above adiminstration
(define sim-priority (make-priority-spec 2 0))

; graphics thread prio, may change
(if claim-visual-thread?
    (define gr-priority (make-priority-spec 1 0))
    (define gr-priority (make-priority-spec 0 0))
    )

; nodes with a different priority scheme
; control loading node has 0, 1 and 2 as above and furthermore
;               3 stick priority
;               4 ticker priority
; priority of the stick. Higher than prio of communication
(define stick-priority (make-priority-spec 3 0)) 

; timing set-up
; timing of the stick calculations. Assuming 500 usec ticks, this gives 2000 Hz
(define stick-timing (make-time-spec 0 1))

; this is normally 20, giving 100 Hz timing
(define sim-timing (make-time-spec 0 20))

;; for now, display on 50 Hz
(define display-timing (make-time-spec 0 80))

;; log a bit more economical, 25 Hz
(define log-timing (make-time-spec 0 80))

;;; the modules needed for dueca itself
(dueca-list
  (make-entity "dueca"
	       (if (equal? 0 this-node-id)
		   (list
		    (make-module 'dusime "" admin-priority)
		    (make-module 'dueca-view "" admin-priority)
		    (make-module 'activity-view "" admin-priority)
		    (make-module 'timing-view "" admin-priority)
		    (make-module 'log-view "" admin-priority)
		    ) 
		 )
	       (list) 
	       )
  )

(define font-manager '())
(if (not (equal? ecs-node this-node-id))
    (set! font-manager
	  (make-font-manager "ftgl"
			     'set-drawing-method "polygon+outline"
			     'add-font (make-font "/usr/share/HMIlib/fonts/Vera.ttf")
			     'add-font (make-font "/usr/share/HMIlib/fonts/Monaco.ttf")
			     'add-font (make-font "/usr/share/HMIlib/fonts/btype.ttf")
			     )
	  )
    )
;;; the modules for your application(s)
(define plib
  (make-entity "plib"
	       (if (equal? ecs-node this-node-id) 
		   (list
		    ;; a module that generates fake input data
		    (make-module 'visual-test-drive "dum" sim-priority
				 'set-timing sim-timing
				 'check-timing 10000 20000
				 'add-motion "myself"
				 'speed 0.6 0 0
				 'position 0 0 -5
				 'dt 0.1
				 'rotation 0 0 0.4
				 'add-motion "house"
				 'set-class "c172"
				 'position 50 0 -2
				 'rotation  1 0 0
				 'dt 0.1
				 'add-motion "rocks"
				 'set-class "c172"
				 'position 30 0 -2
				 'rotation  -1 0 0
				 'dt 0.1
				 )

		    (make-module 'control-view "" admin-priority)
		    )
		   (list)
		   )
	       
	       (if (equal? ig-node-left this-node-id)
	       ;; example ogre application
		   (list
		    (make-module 'world-view "front" gr-priority
				 'claim-thread claim-visual-thread?
				 'set-timing display-timing
				 'check-timing 10000 20000
				 
				 'set-viewer
				 (make-plib-viewer-dueca
				  
				  'sync-divisor 1 
				  'sync-offset 0

				  'set-modelpath 
				  "../../../PlibViewer/data"
				  'set-texturepath
				  "../../../PlibViewer/data"
				 'window-x-screen ":0.0"
				  
				  'add-window "front"
				  'window-size+pos 1858 1080 0 0
				  ;'window-size+pos 800 300 10 10
				 
				  ;; one viewport for the front window
				  'add-viewport "front"
				  'viewport-window "front"
				  'viewport-pos+size 62 0 1796 1080
				  'set-frustum 1.0 30000 
				  -0.4209   ;; left
				   0.8658   ;; right
				  -0.3507   ;; bottom
				   0.4612   ;; top
				  'eye-offset 0 0 0 0 0 0

					; skydome has a diameter of 34000 m, is 10170 m
					; high (realistic to a few 1000 m flight)
					; will stay centered on observer
				  'load-object "sky" "sky.ac"
				  'object-class "centered"
				  'object-coordinates 0.0 0.0 1.0E-10
				 
				 ; ground plane is 36x36 km. has a tiled texture
				 ; with tiles of 2000x2000 m, will be centered 
				 ; with 2000 m jumps
				 'load-object "ground" "ground.ac"
				 'object-class "tiled"
				 'object-coordinates 2000.0 2000.0 0.0

				 ; runway starts at 0 0
				 'load-object "runway" "runway.ac"
				 'object-class "static"
				 
				 ; only type of visible plane
				 'add-object-class "c172" "c172p.ac"

;; 				 'load-object "display"
;; 				 'object-type "displayrate"
;; 				 'object-coordinates 0

				 'load-object "mask1" "hmilabmaskfront-blur.png"
				 'object-class "overlay"
				 'object-coordinates 0
				 )
				)
		    )
		   )

	       (if (equal? ig-node-right this-node-id)
		   (list
		    (make-module 'world-view "sides" gr-priority
				 'claim-thread claim-visual-thread?
				 'set-timing display-timing
				 'check-timing 16000 18000
				 
				 'set-viewer
				 (make-plib-viewer-dueca
				  'sync-divisor 1 
				  'sync-offset 0
				  
				  'set-modelpath 
				  "../../../PlibViewer/data"
				  'set-texturepath
				  "../../../PlibViewer/data"
				  
		      'add-window "sides"
		      'window-size+pos 3520 1080 0 0
		      
		      
		      'add-viewport "left"
		      'viewport-window "sides"
		      'viewport-pos+size 131 0 1719 1080
		      'set-frustum 1.0 30000
		      -0.2796 ;; left
		      1.9294 ;; right
		      -0.6376 ;; bottom
		      0.8057 ;; top
		      'eye-offset 0 0 0 0 0 -82.362

		      'add-viewport "right"
		      'viewport-window "sides"
		      'viewport-pos+size 2172 0 1345 1080
		      'eye-offset 0 0 0 0 0 85.113
		      'set-frustum 1.0 30000
		      -0.8730 ;; left
		       0.1779 ;; right
		      -0.3740 ;; bottom
		       0.4726 ;; top
				 
				 ; skydome has a diameter of 34000 m, is 10170 m
				 ; high (realistic to a few 1000 m flight)
				 ; will stay centered on observer
				 'load-object "sky" "sky.ac"
				 'object-class "centered"
				 'object-coordinates 0.0 0.0 1.0E-10
				 
				 ; ground plane is 36x36 km. has a tiled texture
				 ; with tiles of 2000x2000 m, will be centered 
				 ; with 2000 m jumps
				 'load-object "ground" "ground.ac"
				 'object-class "tiled"
				 'object-coordinates 2000.0 2000.0 0.0

				 ; runway starts at 0 0
				 'load-object "runway" "runway.ac"
				 'object-class "static"

				 'add-object-class "c172" "c172p.ac"

				 'load-object "display"
				 'object-class "displayrate"
				 'object-coordinates 0

 				 'load-object "mask2" "hmilabmaskleft-blur.png"
 				 'object-class "overlay"
 				 'object-coordinates 0

				; 'load-object "mask3" "hmilabmaskright-blur.png"
 				; 'object-class "overlay"
 				; 'object-coordinates 1 

				 )
		    )
		 )
		   )

               ; an empty list; at least one list should be supplied
               ; for nodes that have no modules as argument
	       (list)
	       )
  )

