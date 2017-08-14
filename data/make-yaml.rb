#! /usr/bin/env ruby

require 'open3'
require 'yaml'

file = ARGV[0]
out_file = ARGV[1]

if !File.file?(file)
  abort("file " + ARGV[0] + " does not exist")
end

if out_file == nil
  abort("import <in file> <out file>")
end

awk_for_xs = 'awk \'{print $1}\' ' + file
awk_for_ys = 'awk \'{print $2}\' ' + file

stdin, stdout, stderr  = Open3.popen3(awk_for_xs)

xs = []

stdout.each do |datum|
  xs << datum.to_f
end

stdin, stdout, stderr  = Open3.popen3(awk_for_ys)

ys = []

stdout.each do |datum|
  ys << datum.to_f
end

file_needed = file.split('/')[-1]

region = file_needed.split('-')[0..2].join('-')

availability_zone = file_needed.split('-')[3..5].join('-')

instance_type = file_needed.split('-')[-1].split('.')[0..-2].join('.')

store_in_database = {
  :region => region,
  :availability_zone => availability_zone,
  :instance_type => instance_type,
  :xs => xs,
  :ys => ys
}

File.open(out_file, 'w') do |f|
  f.write store_in_database.to_yaml
end
