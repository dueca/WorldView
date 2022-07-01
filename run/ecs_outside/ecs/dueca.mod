;; -*-scheme-*-

;; set up switches
(define claim-ogre-thread? #t)

;; in general, it is a good idea to clearly document your set up
;; this is an excellent place. 
;; node set-up
(define ecs-node 0)    ; dutmms1, send order 3
(define outside-node 1)

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
(if claim-ogre-thread?
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
(define display-timing (make-time-spec 0 40))

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
		 (list) 
		 )
	       )
  )

;;; the modules for your application(s)
(define ogre
  (make-entity "ogre"
	       (if (equal? outside-node this-node-id) 
		   (list
		    ;; a module that generates fake input data
		    (make-module 'visual-test-drive "" sim-priority
				 'set-timing sim-timing
				 'check-timing 10000 20000
				 'add-motion "myself"
				 'speed 0.6 0 0
				 'dt 0.1
				 'rotation 0 0 0.4
				 'add-motion "house"
				 'position 30.5 0 0
				 'rotation  1 0 0
				 'dt 0.1
				 )

		    ;; example ogre application
		    (make-module 'world-view "" gr-priority
				 'claim-thread claim-ogre-thread?
 				 'set-timing display-timing
 				 'check-timing 10000 20000

				 ;; left window
				 'add-window "left"
				 'window-size+pos 800 300 10 10
				 
				 ;; two viewports for the left window
				 'add-viewport "left side"

				 'viewport-window "left"
				 'viewport-pos+size 0 0 400 300
				 'set-frustum 1.0 1000 0.08 1.58 0.55 -0.55
				 'eye-offset 0 0 0 0 0 -90
				 'viewport-overlay "HMILabOverlays/MaskLeftSide"

				 'add-viewport "left front"
				 'viewport-window "left"
				 'viewport-pos+size 400 0 400 300
				 'set-frustum 1.0 1000 -0.65 0.3 0.356 -0.356
				 'eye-offset 0 0 0 0 0 0
				 'viewport-overlay "HMILabOverlays/MaskLeftFront
"
 
				 ;; right window
				 'add-window "right"
				 'window-size+pos 800 300 10 310

				 'add-viewport "right front"
				 'viewport-window "right"
				 'viewport-pos+size 0 0 400 300
				 'set-frustum 1.0 1000 0.035 0.985 0.356 -0.356
				 'eye-offset 0 0 0 0 0 0
				 'viewport-overlay "HMILabOverlays/MaskRightFron
t"

				 'add-viewport "right side"
				 'viewport-window "right"
				 'viewport-pos+size 400 0 400 300
				 'eye-offset 0 0 0 0 0 90
				 'set-frustum 1.0 1000 -0.915 -0.05 0.363 -0.363
				 'viewport-overlay "HMILabOverlays/MaskRightSide
"				 
				 
				 'initial-camera 0 0 -3 10 0 0

				 'load-scene "sample.scene" 

				 'add-object 
				 (make-dueca-ogre-attachment
				  'name "house")
				 )
		    
		    )
		 )

               ; an empty list; at least one list should be supplied
               ; for nodes that have no modules as argument
	       (list)
	       )
  )

