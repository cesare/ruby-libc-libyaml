# -*- coding: utf-8 -*-
#
# Specs of YAML::LibYAML APIs, focusing on relationship between arguments passed to methods
#   and returned results (or in some cases, exceptions).
#

require File.join(File.dirname(__FILE__), 'spec_helper')

describe YAML::LibYAML do
  
  NON_STRING_OBJECTS = [ nil, 1, 1.23, :filename, ['filename'], {:filename => 'minimum-valid-example.yaml'} ]
  
  #
  # Examples which cause some kind of exceptions, while calling YAML::LibYAML.load_file()
  #
  describe '#load_file' do
    NON_STRING_OBJECTS.each do |obj|
      it "should raise exception, if #{obj.class} objects are passed as arguments" do
        lambda { YAML::LibYAML.load_file(1) }.should raise_error(TypeError)
      end
    end
    
    it 'should raise exception if specified file does not exist' do
      lambda { YAML::LibYAML.load_file('no-such-file.yaml') }.should raise_error(Errno::ENOENT)
    end
  end

  #
  # This is minimum example, just to check if #load_file can parse valid YAML files.
  # See construct_spec.rb for parsing and construction details.
  #
  describe '#load_file' do
    it 'should open specified file and parse its contents into Ruby objects' do
      results = YAML::LibYAML.load_file(get_pathname('minimum-valid-example.yaml'))
      results.should == 123
    end
  end
  

  #
  # Examples which cause some kind of exceptions, while calling YAML::LibYAML.load()
  #
  describe '#load' do
    NON_STRING_OBJECTS.each do |obj|
      it "should raise exception, if #{obj.class} objects are passed as arguments" do
        lambda { YAML::LibYAML.load_file(1) }.should raise_error(TypeError)
      end
    end
  end
  
  #
  # These are minimum examples, just to check if #load can parse valid YAML expressions.
  # See construct_spec.rb for parsing and construction details.
  #
  describe '#load' do
    it 'should parse strings passed as an argument, into Ruby objects' do
      YAML::LibYAML.load('123').should == 123
    end
    
    it 'should read stream and parse its contents into Ruby objects' do
      open(get_pathname('minimum-valid-example.yaml')) { |io| YAML::LibYAML.load(io) }.should == 123
    end
  end

  describe '#dump' do
    it 'should dump nil as undef' do
      input = nil
      result = YAML::LibYAML.dump(input)
      result.should == "--- ~\n...\n"
      YAML::LibYAML.load(result).should == input
    end

    it 'should dump string as scalar' do
      input = ''
      result = YAML::LibYAML.dump(input)
      result.should == "--- \n...\n"
      YAML::LibYAML.load(result).should == input
    end

    it 'should dump "~" as scalar' do
      input = '~'
      result = YAML::LibYAML.dump(input)
      result.should == %{--- "~"\n}
      YAML::LibYAML.load(result).should == input
    end

    it 'should dump string as scalar' do
      input = 'abc'
      result = YAML::LibYAML.dump(input)
      result.should == "--- abc\n...\n"
      YAML::LibYAML.load(result).should == input
    end

    it 'should dump multibyte string as scalar' do
      input = 'あいうえお'
      result = YAML::LibYAML.dump(input)
      result.should == "--- あいうえお\n...\n"
      YAML::LibYAML.load(result).should == input
    end

    it 'should dump array as mapping' do
      input = %w[ foo bar baz ]
      result = YAML::LibYAML.dump(input)
      result.should == <<-END_OF_MAPPING
---
- foo
- bar
- baz
      END_OF_MAPPING
      YAML::LibYAML.load(result).should == input
    end

    it 'should dump nested array as mapping' do
      input = [ %w[ foo bar baz ], %w[ bar baz foo ] ]
      result = YAML::LibYAML.dump(input)
      result.should == <<-END_OF_MAPPING
---
- - foo
  - bar
  - baz
- - bar
  - baz
  - foo
      END_OF_MAPPING
      YAML::LibYAML.load(result).should == input
    end
  end
end
