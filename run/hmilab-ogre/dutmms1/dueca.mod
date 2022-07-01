;; -*-scheme-*-

;; set up switches
(define claim-visual-thread? #t)

;; in general, it is a good idea to clearly document your set up
;; this is an excellent place. 
;; node set-up
(define ecs-node 0)    ; dutmms1, send order 0
(define ig-node-front 1) ; dutmms3_0, send order 1
(define ig-node-sides 2) ; dutmms3_1, send order 2

;; priority set-up
;               0 administration
;               1 ogre graphics drawing thread
;               2 simulation, unpackers
;               3 communication
;               4 ticker

; administration priority. Run the interface and logging here
(define admin-priority (make-priority-spec 0 0))

; priority of simulation, just above adminstration
(if (equal? 0 this-node-id)
    (define sim-priority (make-priority-spec 1 0))
    (define sim-priority (make-priority-spec 2 0))
    )

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

;;; the modules for your application(s)
(define ogre
  (make-entity "ogre"
	       (if (equal? ecs-node this-node-id) 
		   (list
		    ;; a module that generates fake input data
		    (make-module 'visual-test-drive "c" sim-priority
				 'set-timing sim-timing
				 'check-timing 10000 20000
				 'add-motion "myself"
				 'speed 0.6 0 0
				 'dt 0.1
				 'rotation 0 0 0.4
				 'add-motion "house"
				 'set-class "houseX"
				 'position 30.5 0 0
				 'rotation  1 0 0
				 'dt 0.1
				 )
		    
		    (make-module 'control-view "" admin-priority)
		    (make-module 'configure-view "" admin-priority)
		    )
		   (list)
		   )
	       
	       (if (equal? ig-node-front this-node-id)
	       ;; example ogre application
		   (list
		    (make-module 'world-view "front" gr-priority
				 'claim-thread claim-visual-thread?
 				 'set-timing display-timing
 				 'check-timing 10000 20000
				 ;'keep-cursor #t

				 'set-viewer
				 (make-ogre-viewer-dueca
				  
				  ; the ogre viewer loads all its stuff from 
				  ; resources.cfg

				  ;; left window
				  'add-window "front"
				  'window-size+pos 1800 1080 60 0
				  ;'window-size+pos 800 300 10 10
				 
				  ;; one viewport for the front window, left seat
				  'add-viewport "front"
				  'viewport-window "front"
				  'viewport-pos+size 62 0 1796 1080
				  'set-frustum 1.0 10000 
				  -0.4209   ;; left
				   0.8658   ;; right
				  -0.3507   ;; bottom
				   0.4432   ;; top
				  'viewport-overlay "HMILabOverlays/MaskFront"
				  'eye-offset 0 0 0 0 0 0

		  'add-resource-location "." "FileSystem" "local"
				 'load-scene "./sample2.scene" "local"
				 'add-object-class "houseX" "Barrel.mesh"
				 )
;				 'initial-camera 0 0 -3 10 0 0
				 
				 )
		    )
		   )
	       
	       (if (equal? ig-node-sides this-node-id)
		   ;; example ogre application
		   (list
		    (make-module 
		     'world-view "sides" gr-priority
		     'claim-thread claim-visual-thread?
		     'set-timing display-timing
		     'check-timing 10000 20000
		     
		     'set-viewer
		     (make-ogre-viewer-dueca
		      ;'keep-cursor #t
		      'add-window "sides"
		      'window-size+pos 3840 1080 0 0
		      
		      'add-viewport "right"
		      'viewport-window "sides"
		      'viewport-pos+size 2175 0 1345 1080
		      'eye-offset 0 0 0 0 0 85.113
		      'set-frustum 1.0 10000
		      -0.8730 ;; left
		       0.1779 ;; right
		      -0.3740 ;; bottom
		       0.4726 ;; top
		      'viewport-overlay "HMILabOverlays/MaskRightSide"

		      'add-viewport "left"
		      'viewport-window "sides"
		      'viewport-pos+size 121 0 1719 1080
		      'set-frustum 1.0 10000
		      -0.2796 ;; left
		      1.9294 ;; right
		      -0.6376 ;; bottom
		      0.8057 ;; top
		      'eye-offset 0 0 0 0 0 -82.362
		      'viewport-overlay "HMILabOverlays/MaskLeftSide"
		      
		      'add-resource-location "." "FileSystem" "local"
		      'load-scene "./sample2.scene" "local"
		      'add-object-class "houseX" "Barrel.mesh"
		      
		      )
		     
		     
;				 'initial-camera 0 0 -3 10 0 0
				 
		     )
		    )
		   )

	       
               ; an empty list; at least one list should be supplied
               ; for nodes that have no modules as argument
	       (list)
	       )
  )

