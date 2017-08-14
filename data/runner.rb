#! /usr/bin/env ruby

# the file $(pwd)/count_file.txt is used
# to keep track of the number of 
# 'tasks' or regions which have completed

count_file_path = File.join(Dir.pwd, 'count_file.txt').to_s

if !File.exist?(count_file_path)
  open(count_file_path, 'wb') { |f| f.write("") }
end

count = 1.0/0 # Infinity
max = 0

while true do
  while count < max do
    puts('Number tasks completed: ' + count.to_s)
    sleep(60)
    open(count_file_path, 'rb') { |f| count = f.read.split("\n").length }
  end 
  count = 0
  open(count_file_path, 'wb') { |f| f.write("") }
  max = 0
  threads = []
  `aws ec2 describe-regions | grep us | awk '{print $3}'`.split("\n").each do |region|
    puts region
    max += 1
    threads << Thread.new do 
      `./get-region-data.sh #{region}`
      `dpgstarter /mnt/data/receivers.json #{region}` 
    end
  end
  threads.each { |th| th.join }
end
