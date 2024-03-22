# -*- coding: utf-8 -*-
"""
Created on Wed Aug 10 17:12:41 2016

@author: repa

Calculates the frustum parameters for the HMI lab, based on a 2.2 m
projection height, and approx 20 pixel overlap in the projections for
gradual masking

Parameters are significantly different from the old openoffice calc
calculator!

with suggestions from http://geomalgorithms.com/
"""
from lxml import etree
from frustumcalc import eye_right, Segment, frustum_calc, \
    screen_ll, screen_fl, screen_fr, screen_rr, proj_width, res_width, p_height

def writexml(winname, vpx, vpw, fl, fr, fb, ft, vh):
    root = etree.Element('PropertyList')
    sim = etree.SubElement(root, 'sim')
    rendering = etree.SubElement(sim, 'rendering')
    camera_group = etree.SubElement(rendering, 'camera-group')

    window = etree.SubElement(camera_group, 'window')
    l = etree.SubElement(window, 'name')
    l.text = winname
    l.set("type", "string")
    l = etree.SubElement(window, 'host-name')
    l.set("type", "string")
    l = etree.SubElement(window, 'display')
    l.text = '0'
    l = etree.SubElement(window, 'screen')
    l.text = '0'
    l = etree.SubElement(window, 'width')
    l.text = "1920"
    l = etree.SubElement(window, 'height')
    l.text = "1080"
    l = etree.SubElement(window, 'fullscreen')
    l.set('type', "bool")
    l.text = "true"

    camera = etree.SubElement(camera_group, 'camera')
    window = etree.SubElement(camera, 'window')
    l = etree.SubElement(window, 'name')
    l.text = winname

    viewport = etree.SubElement(camera, 'viewport')
    l = etree.SubElement(viewport, 'x')
    l.text = str(vpx)
    l = etree.SubElement(viewport, 'y')
    l.text = "0"
    l = etree.SubElement(viewport, 'width')
    l.text = str(vpw)
    l = etree.SubElement(viewport, 'height')
    l.text = "1080"

    frustum = etree.SubElement(camera, 'frustum')
    l = etree.SubElement(frustum, 'left')
    l.text = str(fl)
    l = etree.SubElement(frustum, 'right')
    l.text = str(fr)
    l = etree.SubElement(frustum, 'bottom')
    l.text = str(fb)
    l = etree.SubElement(frustum, 'top')
    l.text = str(ft)
    l = etree.SubElement(frustum, 'near')
    l.text = "1.0"
    l = etree.SubElement(frustum, 'far')
    l.text = "60000"

    view = etree.SubElement(camera, 'view')
    l = etree.SubElement(view, 'heading-deg')
    l.set('type', "double")
    l.text = str(vh)

    gui = etree.SubElement(camera_group, "gui")
    window = etree.SubElement(gui, 'window')
    l = etree.SubElement(window, 'name')
    l.set("type", "string")
    l.text = winname

    etree.ElementTree(root).write(f'hmilab-camera-{winname}.xml', pretty_print=True,
                xml_declaration=True, encoding='utf-8')

# left screen
width_l = Segment(point0=screen_ll, point1=screen_fl)
pixels_l = int(round(width_l.length / proj_width * res_width))
fr, offs = frustum_calc(screen_ll, screen_fl, screen_fl + p_height, eye_right, 1.0)
writexml("left", 130, pixels_l, *fr, offs)

width_f = Segment(point0=screen_fl, point1=screen_fr)
pixels_f = int(round(width_f.length / proj_width * res_width))
fr, offs = frustum_calc(screen_fl, screen_fr, screen_fr + p_height, eye_right, 1.0)
writexml("front", 120, pixels_f, *fr, offs)

width_r = Segment(point0=screen_fr, point1=screen_rr)
pixels_r = int(round(width_r.length / proj_width * res_width))
fr, offs = frustum_calc(screen_fr, screen_rr, screen_rr + p_height, eye_right, 1.0)
writexml("right", 240, pixels_r, *fr, offs)
