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

from matplotlib import pyplot as plt
import numpy as np

import warnings
try:
    warnings.filterwarnings("ignore", category=np.VisibleDeprecationWarning)
except AttributeError:
    pass

__eps = 1e-12


class UnitVector(np.matrix):
    def __new__(cls, *args):
        if len(args) == 3:
            self = super(UnitVector, cls).__new__(cls, args)
        elif len(args) == 1:
            self = super(UnitVector, cls).__new__(cls, args[0])
        else:
            return None
        self /= np.linalg.norm(self)
        return self


class Point(np.matrix):
    def __new__(cls, *args):
        if len(args) == 3:
            return super(Point, cls).__new__(cls, args)
        else:
            return super(Point, cls).__new__(cls, args[0])

    def distance(self, other):
        if isinstance(other, Point):
            return np.linalg.norm(other - self)
        return other.distance(self)

    def project(self):
        return Point(self)


class Line(object):
    def __init__(self, **kwargs):
        """ Create an infinite length line

            Call by specifying either point0 and point1, or by
            specifying point0 and direction.

            Parameters
            ----------
            point0: 3-element vector/iterable
                One point on the line
            point1: 3-element vector/iterable
                Another point on the line
            direction: UnitVector
                Direction of the line
        """
        self.orig = Point(kwargs['point0'])
        try:
            self.direction = UnitVector(kwargs['direction'])
        except KeyError:
            self.direction = UnitVector(kwargs['point1'] - self.orig)

    def project(self, other):
        if isinstance(other, Point):
            p_dist = np.linalg.norm(self.direction, other - self.orig)
            return Point(self.orig + self.direction*p_dist)
        raise TypeError()

    def parallel(self, other):
        global __eps
        if isinstance(other, Line):
            return np.linalg.norm(np.cross(self.direction,
                                           other.direction)) < __eps
        raise TypeError()

    def crossing(self, other):
        if isinstance(other, Line):
            # http://geomalgorithms.com/a07-_distance.html
            b = np.inner(self.direction, other.direction)[0, 0]
            d = np.inner(self.direction, self.orig - other.orig)[0, 0]
            e = np.inner(other.direction, self.orig - other.orig)[0, 0]
            s_self = (b*e-d)/(1-b*b)
            s_other = (e - b*d)/(1-b*b)
            return Segment(point0=self.orig+self.direction*s_self,
                           point1=other.orig+other.direction*s_other)
        raise TypeError()

    def distance(self, other):
        if isinstance(other, Point):
            return np.linalg.norm(other - self.project(other))
        elif isinstance(other, Line) and not self.parallel(other):
            plane = (self.orig, np.outer(self.direction, other.direction))
            return plane.distance(other.orig)
        raise TypeError()


class Segment(Line):
    def __init__(self, **kwargs):
        """ Create a limited-length line

        Call by specifying the line's begin and end point"""
        super(Segment, self).__init__(**kwargs)
        self.length = np.linalg.norm(kwargs['point1']-kwargs['point0'])

    def project(self, other):
        if isinstance(other, Point):
            p_dist = np.linalg.norm(self.direction, other - self.orig)
            if p_dist < 0.0 or p_dist > self.length:
                return ValueError()
            return Point(self.orig + self.direction*p_dist)
        raise TypeError()

    def distance(self, other):
        if isinstance(other, Point):
            try:
                return np.linalg.norm(self.project(other) - other)
            except ValueError:
                return min(
                    (np.linalg.norm(self.orig - other),
                     np.linalg.norm(self.orig+self.length*self.direction
                                    - other)))


class Cylinder(Line):
    def __init__(self, line, radius):
        super(Cylinder, self).__init__(point0=line.orig, direction=line.direction)
        self.radius = radius

    def project(self, other):
        pnt = super(Cylinder, self).project(other)
        u_vect = UnitVector(other-pnt)
        return Point(pnt + self.radius*u_vect)

    def distance(self, other):
        return super(Cylinder, self).distance(other) - self.radius

    def intersect(self, other):
        if isinstance(other, Cylinder):
            raise TypeError('not implemented')
        if isinstance(other, Line):
            crossing = other.crossing(self)
            if crossing.length > self.radius:
                return None
            d_points = np.sqrt(self.radius**2 - crossing.length**2) / \
                np.linalg.norm(np.cross(self.direction, other.direction))
            return (crossing.orig - other.direction*d_points,
                    crossing.orig + other.direction*d_points)


class LimitedCylinder(Cylinder):
    def __init__(self, line, radius):
        super(LimitedCylinder, self).__init__(line, radius)

    def distance(self, other):
        raise TypeError()


class Plane(np.matrix):
    def __new__(cls, **kwargs):
        try:
            # from point and normal vector
            normal = UnitVector(kwargs['normal'])
            dist = float(np.inner(normal, kwargs['point0'])[0,0])
        except KeyError:
            # from three points, positive clockwise?
            normal = UnitVector(np.cross(
                kwargs['point1'] - kwargs['point0'],
                kwargs['point2'] - kwargs['point1']))
            dist = float(np.inner(normal, kwargs['point0'])[0,0])
#        print([float(normal[0, 0]), float(normal[0, 1]), float(normal[0, 2]), -dist])
        return super(Plane, cls).__new__(
            cls, [float(normal[0,0]), float(normal[0,1]),
                  float(normal[0,2]), -float(dist)])

    def distance(self, other):
        if isinstance(other, Point):
            return np.inner(self, np.append(other,
                                            np.matrix(1.0), axis=1))[0, 0]

    def normal(self):
        return np.asarray(self[:, :3])

    def intersect(self, other):
        if isinstance(other, Line):
            s = np.inner(self, np.append(other.orig,
                                         np.matrix(1.0), axis=1)) / \
                np.inner(other.direction, self.normal())
            return Point(other.orig - s*other.direction)
        elif isinstance(other, Plane):
            direction = np.cross(self.normal(), other.normal())
            line = Line(point0=self.project(Point(0, 0, 0)),
                        point1=self.project(Point(other.normal())))
            return Line(point0=other.intersect(line),
                        direction=direction)

    def project(self, other):
        if isinstance(other, Point):
            return Point(other - self.normal()*self.distance(other))
        else:
            raise TypeError("cannot project class %s" % other.__class__.__name__)


def frustum_calc(p0, p1, p2, pv, f_dist):
    """Calculate frustum parameters, from 3 point plane
        (left btm, right btm, right top), view point and frustum distance"""

    # create a plane for the projection and a left/bottom plane
    plane_p = Plane(point0=p0, point1=p1, point2=p2)
    plane_l = Plane(point0=p0, normal=p1-p0)
    plane_b = Plane(point0=p0, normal=p2-p1)
    plane_r = Plane(point0=p1, normal=p0-p1)
    plane_t = Plane(point0=p2, normal=p1-p2)
    # print plane_p, pv

    # calculate position of viewpoint in frustum
    dist = plane_p.distance(pv)
    frustum = (-plane_l.distance(pv)/dist*f_dist,
                plane_r.distance(pv)/dist*f_dist,
               -plane_b.distance(pv)/dist*f_dist,
                plane_t.distance(pv)/dist*f_dist)
    angle = -np.arctan2(plane_p.normal()[0, 0], -plane_p.normal()[0, 1]) * 180/np.pi

    print(f"""
                ('set-frustum',
                 ({f_dist}, 10000.0,
                  {frustum[0]:9.6f}, {frustum[1]:9.6f},
                  {frustum[2]:9.6f}, {frustum[3]:9.6f})),
                ('eye-offset',
                 (0.0, 0.0, 0.0, 0.0, 0.0, {angle:9.6f})),""")

    return frustum, angle

# lab dimensions,  origin, left rear, left front, right front (meter)
lab_width = 4.687
lab_depth = 4.25

lab_lr = Point((0, 0, 0))
lab_lf = Point((0, lab_depth, 0))
lab_rf = Point((lab_width, lab_depth, 0))

# projection_resolution (pixel)
res_width = 1920
res_height = 1080

# size of a full projection
proj_height = 2.24
proj_width = proj_height / res_height * res_width
proj_bottom = 0.675

# left eye(left seat), right eye(right seat)
# eye_left = Point((1.73, lab_depth-2.85, 0.7+0.95))
# a bit higher, test image was deformed.
eye_left = Point((1.73, lab_depth-2.85, 1.85))
eye_right = Point((1.73+1.05, lab_depth-2.85, 1.85))

# front screen, physical size
# measure from concrete wall to intersection both canvas
screen_fl = Point((0.59, lab_depth-0.11, proj_bottom))
screen_fr = Point((lab_width-0.59, lab_depth-0.11, proj_bottom))
p_height = Point((0, 0, proj_height))

# left screen, connects to front (screen_fl)
# measurement from concrete wall to seam canvas
screen_ll = Point((0.1, 0.79, proj_bottom))

# right screen, connects to front (screen_fr)
screen_rr_compensation = 0.07
screen_rr = Point((lab_width-0.32, 1.52+screen_rr_compensation, proj_bottom))

# left screen width
width_l = Segment(point0=screen_ll, point1=screen_fl)
pixels_l = int(round(width_l.length / proj_width * res_width))
print("Left screen width:", width_l.length)
print("Left screen pixels:", pixels_l, "with offset:", 140)
print("Left screen pixels with blend:", pixels_l + 20, "with offset: ", 140)
print("")

# left screen bleed
bleed_l = proj_width * (pixels_l+20)/res_width - width_l.length
# left screen virtual end point
screen_lrv = screen_ll + (screen_fl - screen_ll) * \
     (width_l.length + bleed_l) / width_l.length
# print "left l=", screen_ll, "r=", screen_fl, "v=", screen_lrv

# front screen width
width_f = Segment(point0=screen_fl, point1=screen_fr)
# front screen pixels, 2x bleed
pixels_f = int(round(width_f.length / proj_width * res_width))
print("Front screen width:", width_f.length)
print("Front screen pixels:", pixels_f, "with offset: ", 120)
print("Front screen pixels with blend:", pixels_f + 40, "with offset: ", 100)
print("")
bleed_f = proj_width * (pixels_f+40)/res_width - width_f.length
# print(bleed_f/2, 20/1080*2.24)

# front screen virtual end points
screen_flv = screen_fl - (screen_fr - screen_fl) * \
    (bleed_f/2)/width_f.length
screen_frv = screen_fr + (screen_fr - screen_fl) * \
    (bleed_f/2)/width_f.length
# print("front vl=", screen_flv, "l=", screen_fl, "r=", screen_fr, "vr=", screen_frv)

# right screen width
width_r = Segment(point0=screen_fr, point1=screen_rr)
pixels_r = int(round(width_r.length / proj_width * res_width))
print("Right screen width:", width_r.length)
print("Right screen pixels:", pixels_r, "with offset: ", 260)
print("Right screen pixels with blend:", pixels_r + 20, "with offset: ", 240)
print("")

# right screen bleed
bleed_r = proj_width * (pixels_r+20)/res_width - width_r.length

# right screen virtual start point
screen_rlv = screen_fr - (screen_rr - screen_fr) * \
             bleed_r / width_r.length
# print( "right lv=", screen_rlv, "l=", screen_fr, "r=", screen_rr)

print("Frustum values right seat")
print("left screen frustum, right seat")
frustum_calc(screen_ll, screen_lrv, screen_lrv+p_height, eye_right, 1.0)
print("front screen frustum, right seat")
frustum_calc(screen_flv, screen_frv, screen_frv+p_height, eye_right, 1.0)
print("right screen frustum, right seat")
frustum_calc(screen_rlv, screen_rr, screen_rr+p_height, eye_right, 1.0)
print("")
print("Frustum values left seat")
print("left screen frustum, left seat")
frustum_calc(screen_ll, screen_lrv, screen_lrv+p_height, eye_left, 1.0)
print("front screen frustum, left seat")
frustum_calc(screen_flv, screen_frv, screen_frv+p_height, eye_left, 1.0)
print("right screen frustum, left seat")
frustum_calc(screen_rlv, screen_rr, screen_rr+p_height, eye_left, 1.0)
