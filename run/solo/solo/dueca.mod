;; -*-scheme-*-

;; set up switches
(define claim-ogre-thread? run-in-multiple-threads?)

;; in general, it is a good idea to clearly document your set up
;; this is an excellent place. 
;; node set-up
(define ecs-node 0)    ; dutmms1, send order 3

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
(define gr-priority #f)
(if claim-ogre-thread?
	(set! gr-priority (make-priority-spec 1 0))
	(set! gr-priority (make-priority-spec 0 0))
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

;; log a bit more economical, 10 Hz
(define log-timing (make-time-spec 0 200))

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
			(make-module 'channel-view "" admin-priority)
			) 
		(list) 
		)
		)
)

(define f 4)

;;; the modules for your application(s)
(define driving
  (make-entity "drive"
	(if (equal? ecs-node this-node-id) 
	    (list
	     ;; a module that generates fake input data
	     (make-module 'visual-test-drive "" admin-priority
			  'set-timing sim-timing
			  'check-timing 10000 20000
			  'add-motion "myself"
			  'position -80 0 -30
			  'orientation 0 0 0
			  'speed 1.0 0 0
			  'dt 0.1
			  'rotation 0 0 0.4
			  'add-motion "houseX"
			  'position 0 0 -30
			  'rotation  1 0 0
			  'dt 0.1
			  'add-motion "head"
			  'position 160 40 -40
			  'orientation 0 0 0
			  'rotation 1 1 10
			  'dt 0.1
			  )
	     )
	    )
	)
  )

(define ogre
(make-entity "ogre"
	(if (equal? ecs-node this-node-id) 
		(list
		 (make-module 'world-view "" gr-priority
			      'claim-thread claim-ogre-thread?
			      'set-timing display-timing
			      'check-timing 10000 20000
			      
                              'set-viewer
                              (make-ogre-viewer-dueca
					; the ogre viewer loads all its stuff from 
					; resources.cfg
                               'render-shadows #t
                               'set-shadow-technique "Stencil"
                               'set-shadow-far-distance 100
                               'set-shadow-colour 1.0 1.0 1.0
                               
                               ;; front window
                               'add-window "front"
                               'window-size+pos
                               (round (/ 1920 f)) (round (/ 1080 f)) 0 0
                               
                               ;; one viewport for the front window
                               'add-viewport "front"
                               'viewport-window "front"
                               'viewport-pos+size
                               (round (/ 62 f)) 0
                               (round (/ 1796 f)) (round (/ 1080 f))
                               'set-frustum 1.0 10000 
                              ;-0.7845  0.4795 -0.3269  0.4530  ; right seat
                              ; -0.4209  0.8658 -0.3507  0.4432  ; left seat
                               -1.17137785454 0.724227595471 -0.479274611399 0.660621761658
                               
                               'eye-offset 0 0 0 0 0 0
                               'viewport-overlay "HMILabOverlays/MaskFrontSharp"

                               ;; window with side views
                               'add-window "sides"
                               'window-size+pos
                               (round (/ 3840 f)) (round (/ 1080 f)) 0 0
                               
                               'add-viewport "left side"
                               'viewport-window "sides"
                               'viewport-pos+size
                               (round (/ 121 f)) 0
                               (round (/ 1719 f)) (round (/ 1080 f))
                               'eye-offset 0 0 0 0 0 -82.362
                               'set-frustum 1.0 10000
                              ;-0.0925  1.2185 -0.3590  0.4976  ; right seat
                              ; -0.2796  1.9296 -0.6376  0.8057  ; left seat
                               -0.75011761385 0.675155946186 -0.376500155079 0.51895967321
                               'viewport-overlay "HMILabOverlays/MaskLeftSideSharp"
                               
                               'add-viewport "right side"
                               'viewport-window "sides"
                               'viewport-pos+size
                               (round (/ 2175 f)) 0
                               (round (/ 1345 f)) (round (/ 1080 f))
                               'eye-offset 0 0 0 0 0 85.113
                               'set-frustum 1.0 10000
                              ;-1.7224 -0.0027 -0.5806  0.8047  ; right seat
                              ; -0.8730  0.1779 -0.3740  0.4726  ; left seat
                               -1.21819795407 0.588489166461 -0.60996297175 0.840759771872
                               'viewport-overlay "HMILabOverlays/MaskRightSideSharp"
	
                               'add-resource-location "." "FileSystem" "General"
                               'load-scene "./sample2.scene" "General"

                               'add-object-class-data "ObjectMotion"
                               "compatible #" "OgreObjectCompatible"
                               'add-object-class "houseX" "Barrel.mesh"
                               'add-object-class "head" "ogrehead.mesh"
                               'add-object-class-data "manualcreate" "happy"
                               "OgreObjectCarried" "General/Barrel.mesh"
                               'add-object-class-coordinates
                               10.0 0.0 0.0 0.0 0.0 0.0
                               'create-static "manualcreate" "happy"
                               ;; link the ObjectMotion objects
                               )
                              'initial-camera 0 0 -30 0 0 0
                              )
                 
                 (make-module 'configure-view "" admin-priority
                              'set-timing display-timing
                              )
                 
                 (make-module 'control-view "" admin-priority
                              'set-timing display-timing
                              'position 0 -45 -15
                              'orientation 0 -5 51
			)
                 
                 )
                )
        ; an empty list; at least one list should be supplied
	; for nodes that have no modules as argument
	(list)
	)
)

