# -*- coding: utf-8 -*-

require File.join(File.dirname(__FILE__), 'spec_helper')

describe YAML::LibYAML do
  describe '#load' do
    ['.nan', '.NaN', '.NAN'].each do |str|
      it "should interpret '#{str}' as NaN" do
        YAML::LibYAML.load(str).should be_nan
      end
    end
    
    ['.inf', '.Inf', '.INF', '+.inf', '+.Inf', '+.INF'].each do |str|
      it "should interpret '#{str}' as positive infinity" do
        YAML::LibYAML.load(str).should be_infinite
      end
    end

    ['-.inf', '-.Inf', '-.INF'].each do |str|
      it "should interpret '#{str}' as negative infinity" do
        YAML::LibYAML.load(str).infinite?.should == -1
      end
    end
    
    ['~', 'null', 'Null', 'NULL'].each do |str|
      it "should interpret '#{str}' as nil" do
        YAML::LibYAML.load(str).should be_nil
      end
    end
    
    ['true', 'True', 'TRUE'].each do |str|
      it "should interpret '#{str}' as true" do
        YAML::LibYAML.load(str).should be_true
      end
    end
    
    ['false', 'False', 'FALSE'].each do |str|
      it "should interpret '#{str}' as false" do
        YAML::LibYAML.load(str).should be_false
      end
    end

    {
      '010' => 8,
      '0x10' => 16,
      '0x0f' => 15,
      '10' => 10,
      '1'  =>  1,
      '+1' =>  1,
      '-1' => -1,
      '0'  =>  0,
    }.each do |input,expect|
      it "should interpret '#{input}' as #{expect}" do
        YAML::LibYAML.load(input).should == expect
      end
    end
    
    {
      ':symbol'          => :symbol,
      ':123'             => :"123",
      ':test of symbols' => :"test of symbols",
      ':@attribute'      => :"@attribute",
      ':@@cattr'         => :"@@cattr",
      ':$variable'       => :'$variable',
    }.each do |str, expected|
      it "should interpret '#{str}' as a symbol" do
        YAML::LibYAML.load(str).should == expected
      end
    end
  end
end
