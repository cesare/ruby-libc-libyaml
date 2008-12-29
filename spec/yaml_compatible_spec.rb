require File.join(File.dirname(__FILE__), 'spec_helper')

begin
  require 'spec/fixture'
  require 'yaml'

  describe YAML::LibYAML do
    describe ".load" do
      with_fixtures :yaml => :option do

        it "should be compatible to YAML (:msg)" do |yaml,option|
          YAML::LibYAML.load(yaml).should == YAML.load(yaml)
        end

        # FIXME: hmm. are there any cool way to manage simple?
        set_fixtures([
          [ { <<-END_YAML => nil } , "simple mapping" ],
---
key: val
          END_YAML
          [ { <<-END_YAML => nil } , "simple sequense" ],
---
- foo
- bar
- baz
          END_YAML
          [ { <<-END_YAML => nil } , "seq in map" ],
---
global:
  - foo
  - bar
  - baz
          END_YAML
        ])
      end
    end
  end
rescue LoadError => e
  warn "this spec require rspec-fixture."
end
