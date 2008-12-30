require File.join(File.dirname(__FILE__), 'spec_helper')

begin
  begin
    require 'spec/fixture'
  rescue LoadError => e
    require 'rubygems'
    gem 'rspec-fixture'
    require 'spec/fixture'
  end

  require 'yaml'

  describe YAML::LibYAML do
    describe ".load" do
      with_fixtures :yaml => :message do
        it "should be compatible to YAML (:message)" do |yaml,message|
          YAML::LibYAML.load(yaml).should == YAML.load(yaml)
        end

        test_data = <<-EOF_TEST_DATA
=== simple mapping
---
key: val

=== simple sequense
---
- foo
- bar
- baz

=== seq in map
---
global:
  - foo
  - bar
  - baz
        EOF_TEST_DATA


        # making fixture data
        data = test_data.split(/^(===.+)$\n/)
        data.shift # remove garbage

        fixtures = []
        while message = data.shift
          message.gsub!(/^===\s+/, '')
          yaml = data.shift
          fixtures << [ { yaml => message }, message ]
        end

        set_fixtures(fixtures)
      end
    end
  end
rescue LoadError => e
  warn "this spec require rspec-fixture."
end
