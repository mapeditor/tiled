"""
 Python Tiled Plugin
 Copyright 2012, Samuli Tuomola <samuli@tuomola.net>

 This file is part of Tiled.

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along with
 this program. If not, see <http://www.gnu.org/licenses/>.
"""

import re
from pybindgen import *
import pybindgen.typehandlers.base as typehandlers
from pybindgen.typehandlers.base import ForwardWrapperBase, PointerParameter

"""
class QFlagsTransformation(typehandlers.TypeTransformation):
  def __init__(self):
    self.rx = re.compile(r'(?:::)?QFlags<\s*(\w+)\s*>')

  def get_untransformed_name(self, name):
    m = self.rx.match(name)
    if m is None:
      return None
    else:
      return m.group(1)+' *'

  def create_type_handler(self, type_handler, *args, **kwargs):
    ctype = self.get_untransformed_name(args[0])
    handler = type_handler(ctype, *args[1:], **kwargs)
    handler.has_been_transformed = True
    return handler

typehandlers.param_type_matcher.register_transformation(QFlagsTransformation())
"""

class QFlagsOptionParam(Parameter):
  DIRECTIONS = [Parameter.DIRECTION_IN]
  CTYPES = ['QFlags<QFileDialog::Option>']

  def get_c_error_return(self):
    return "return QFlags(0);"

  def convert_c_to_python(self, wrapper):
    raise NotImplementedError
    #wrapper.build_params.add_parameter("s", ['%s.toUtf8().data()' % self.value], prepend=True)

  def convert_python_to_c(self, wrapper):
    name = wrapper.declarations.declare_variable("QFileDialog::Option", self.name)
    wrapper.parse_params.add_parameter('i', ['&'+name], self.value, optional=bool(self.default_value))
    if self.default_value is None:
      wrapper.call_params.append('(QFlags<QFileDialog::Option>)%s' % name)
    else:
      wrapper.call_params.append(self.default_value)

class QStringPtrParam(PointerParameter):
  DIRECTIONS = [Parameter.DIRECTION_IN]	# could be out as well
  CTYPES = ['QString*']

  def convert_c_to_python(self, wrapper):
    raise NotImplementedError

  def convert_python_to_c(self, wrapper):
    name = wrapper.declarations.declare_variable("const char *", self.name)
    if self.default_value is None:
      name_qst = wrapper.declarations.declare_variable("QString*", self.name + '_qst',
        'new QString(%s)' % name)
      wrapper.call_params.append('%s' % name_qst)
    else:
      wrapper.call_params.append(self.default_value)
    wrapper.parse_params.add_parameter('s', ['&'+name], self.value, optional=bool(self.default_value))

class QStringParam(Parameter):
  DIRECTIONS = [Parameter.DIRECTION_IN]
  CTYPES = ['QString']

  def get_c_error_return(self):
    return "return QString();"

  def convert_c_to_python(self, wrapper):
    wrapper.build_params.add_parameter("s", ['%s.toUtf8().data()' % self.value], prepend=True)

  def convert_python_to_c(self, wrapper):
    name = wrapper.declarations.declare_variable("const char *", self.name)
    len_ = wrapper.declarations.declare_variable("Py_ssize_t", self.name+"_len")
    wrapper.parse_params.add_parameter('s#', ['&'+name, '&'+len_], self.value)
    wrapper.call_params.append('QString::fromUtf8(%s)' % name)

class QStringReturnValue(ReturnValue):
  CTYPES = ['QString']

  def get_c_error_return(self):
    return "return QString();"

  def convert_python_to_c(self, wrapper):
    #raise NotImplementedError # TODO (needed only for virtual methods where C calls Python code)
    ptr = wrapper.declarations.declare_variable("const char *", "retval_ptr")
    len_ = wrapper.declarations.declare_variable("Py_ssize_t", "retval_len")
    wrapper.parse_params.add_parameter("s#", ['&'+ptr, '&'+len_])
    wrapper.after_call.write_code("%s = QString(%s);" % (self.value, ptr))

  def convert_c_to_python(self, wrapper):
    wrapper.build_params.add_parameter("s", ['%s.toUtf8().data()' % self.value], prepend=True)

def generate(parent_mod):
  mod = module.SubModule('qt', parent_mod)
  mod.add_include('<QImage>')
  mod.add_include('<QFileDialog>')
  mod.add_include('<QWidget>')
  mod.add_include('<QFlags>')
  #mod.add_include('"qtbind.h"')
  
  cls_qpointf = mod.add_class('QPointF')
  cls_qpointf.add_constructor([('float','x'),('float','y')])
  cls_qpointf.add_method('x', 'int', [])
  cls_qpointf.add_method('setX', None, [('int','x')])
  cls_qpointf.add_method('y', 'int', [])
  cls_qpointf.add_method('setY', None, [('int','y')])
  
  cls_sizef = mod.add_class('QSizeF')
  cls_sizef.add_constructor([('float','w'),('float','h')])
  cls_sizef.add_method('width', 'int', [])
  cls_sizef.add_method('setWidth', None, [('int','w')])
  cls_sizef.add_method('height', 'int', [])
  cls_sizef.add_method('setHeight', None, [('int','h')])
  
  cls_qrgb = mod.add_class('QRgb')
  mod.add_container('QVector<QRgb>', retval('QRgb'), 'vector')
  
  cls_color = mod.add_class('QColor')
  cls_color.add_constructor([('QRgb','col')])
  cls_color.add_constructor([('int','r'), ('int','g'), ('int','b')])
  cls_color.add_constructor([('int','r'), ('int','g'), ('int','b'),('int','a')])
  cls_color.add_method('rgb', 'QRgb', [])
  cls_color.add_method('rgba', 'QRgb', [])
  
  cls_qimage = mod.add_class('QImage')
  cls_qimage.add_enum('Format', ('Format_Invalid','Format_Mono','Format_MonoLSB',
    'Format_Indexed8','Format_RGB32','Format_ARGB32',
    'Format_ARGB32_Premultiplied','Format_RGB16',
    'Format_ARGB8565_Premultiplied','Format_RGB666',
    'Format_ARGB6666_Premultiplied','Format_RGB555',
    'Format_ARGB8555_Premultiplied','Format_RGB888','Format_RGB444',
    'Format_ARGB4444_Premultiplied'))
  cls_qimage.add_constructor([])
  cls_qimage.add_constructor([('int','w'), ('int','h'), ('Format','f')])
  cls_qimage.add_method('color', 'QRgb', [('int','i')])
  cls_qimage.add_method('colorTable', 'QVector<QRgb>', [])
  cls_qimage.add_method('fill', None, [('int','color')])
  cls_qimage.add_method('load', 'bool', [('const QString','fileName'),('char*','fmt')])
  cls_qimage.add_method('mirrored', retval('const QImage&'), [('bool','horiz'),('bool','vert')])
  cls_qimage.add_method('width', 'int', [])
  cls_qimage.add_method('height', 'int', [])
  cls_qimage.add_method('setColor', None, [('int','i'),('QRgb','c')])
  cls_qimage.add_method('setPixel', None, [('int','x'),('int','y'),
    ('unsigned int','color')])
  cls_qimage.add_method('setPixel', None, [('int','x'),('int','y'),
    ('QRgb','color')])
  cls_qimage.add_method('setColorTable', None, [('QVector<QRgb>','colors')])
  cls_qpixmap = mod.add_class('QPixmap')
  cls_qpixmap.add_method('toImage', retval('const QImage&'), [])
  cls_qpixmap.add_method('fromImage', None, [('const QImage&','image')])
  cls_qpixmap.add_method('convertFromImage', None, [('const QImage&','image')])
  cls_qpixmap.add_method('width', 'int', [])
  cls_qpixmap.add_method('height', 'int', [])
  
  cls_qwidget = mod.add_class('QWidget')
  cls_qfiledialog = mod.add_class('QFileDialog')
  cls_qfiledialog.add_enum('Option', ('ShowDirsOnly','DontResolveSymlinks','DontConfirmOverwrite','DontUseNativeDialog',
    'ReadOnly','HideNameFilterDetails','DontUseSheet'))
  cls_qfiledialog.add_method('getOpenFileName', 'QString', [
    param('QWidget*','parent',transfer_ownership=False,null_ok=True),
    ('const QString','caption'),('const QString','dir'),('const QString','filter'),
    param('QString*','selectedFilter',default_value='new QString("")'),
    param('QFlags<QFileDialog::Option>','options', direction=Parameter.DIRECTION_IN, default_value='0')
    ], is_static=True)
  
  mod.add_container('QList<QString>', retval('QString'), 'list')
"""
  with open('qtbind.h','w') as fh:
    import pybindgen.typehandlers.codesink as cs
    sink = cs.MemoryCodeSink()
    mod.generate_forward_declarations(sink)
    includes = ['QImage','QFileDialog','QWidget','Qflags']
    print >>fh, '\n'.join(['#include <%s>' % f for f in includes])
    print >>fh, sink.flush()

  with open('qtbind.cpp','w') as fh:
    mod.generate(fh)
"""
